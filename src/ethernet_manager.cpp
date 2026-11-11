#include "ethernet_manager.h"
#include <WiFi.h>
#include <esp_eth.h>
#include <esp_eth_mac_spi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_mac.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "config/config.h"
#include "hardware.h"

// State Ethernet
static bool eth_connected = false;
static bool eth_link_up = false;
esp_netif_t *eth_netif_ptr = NULL;
static esp_eth_handle_t eth_handle = NULL;

// Event handler untuk Network
static void eth_event_handler(arduino_event_id_t event, arduino_event_info_t info)
{
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("ETH Started");
            break;

        case ARDUINO_EVENT_ETH_CONNECTED:
            eth_link_up = true;
            Serial.println("ETH Connected (Link Up). Memulai DHCP...");
            if (eth_netif_ptr) {
                esp_netif_dhcpc_start(eth_netif_ptr); // error "sudah berjalan" boleh diabaikan
            }
            break;

        case ARDUINO_EVENT_ETH_GOT_IP: {
            eth_connected = true;
            esp_netif_ip_info_t ip = {};
            if (eth_netif_ptr && esp_netif_get_ip_info(eth_netif_ptr, &ip) == ESP_OK) {
                Serial.printf("ETH Got IP: " IPSTR " | GW: " IPSTR " | Mask: " IPSTR "\n",
                              IP2STR(&ip.ip), IP2STR(&ip.gw), IP2STR(&ip.netmask));
            } else {
                Serial.println("ETH Got IP");
            }
            // Matikan WiFi jika Ethernet berhasil konek agar tidak konflik rute & hemat daya
            if (WiFi.getMode() != WIFI_OFF) {
                Serial.println("[Failover] LAN terhubung! Mematikan radio WiFi cadangan & mengalihkan seluruh koneksi ke LAN.");
                WiFi.disconnect(true);
                WiFi.mode(WIFI_OFF);
            }
            break;
        }

        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("ETH Disconnected - Link kabel LAN putus!");
            eth_connected = false;
            eth_link_up = false;
            break;

        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("ETH Stopped");
            eth_connected = false;
            eth_link_up = false;
            break;

        default:
            break;
    }
}

// Task diagnostik satu kali: cek status IP setelah 15 detik
static void eth_diag_task(void *param)
{
    vTaskDelay(pdMS_TO_TICKS(15000));
    if (!eth_connected) {
        esp_netif_ip_info_t ip = {};
        if (eth_netif_ptr) esp_netif_get_ip_info(eth_netif_ptr, &ip);
        Serial.printf("[ETH-DIAG] Belum dapat IP setelah 15 detik. IP saat ini: " IPSTR "\n",
                      IP2STR(&ip.ip));
        Serial.println("[ETH-DIAG] Cek: kabel LAN, DHCP server/ICS aktif, pin W5500.");
    }
    vTaskDelete(NULL);
}

void ethernetSetup()
{
    Serial.println("===== Inisialisasi W5500 via ESP-IDF =====");

    // 1. TCP/IP core + event loop (aman dipanggil ulang)
    esp_err_t r = esp_netif_init();
    if (r != ESP_OK) {
        Serial.printf("Peringatan esp_netif_init: %s\n", esp_err_to_name(r));
    }
    r = esp_event_loop_create_default();
    if (r != ESP_OK && r != ESP_ERR_INVALID_STATE) {
        Serial.printf("Peringatan event loop: %s\n", esp_err_to_name(r));
    }

    // 2. Install GPIO ISR service (wajib untuk pin INT W5500)
    esp_err_t ret_isr = gpio_install_isr_service(0);
    if (ret_isr != ESP_OK && ret_isr != ESP_ERR_INVALID_STATE) {
        Serial.printf("Peringatan install isr service: %s\n", esp_err_to_name(ret_isr));
    }

    // 3. Daftarkan event handler
    WiFi.onEvent(eth_event_handler);

    // 4. Hardware reset W5500 manual
    pinMode(W5500_RST, OUTPUT);
    digitalWrite(W5500_RST, LOW);
    delay(10);
    digitalWrite(W5500_RST, HIGH);
    // Beri waktu chip siap. 50ms sudah cukup (datasheet: PLL lock ~1ms).
    // JANGAN gunakan delay besar (misal 500ms) karena PHY bisa selesai negosiasi
    // SEBELUM driver sempat mendaftarkan interrupt handler, sehingga event Link Up pertama terlewat!
    delay(50);

    // 5. Buat netif Ethernet (DHCP client otomatis) dengan prioritas rute lebih tinggi dari WiFi (150 vs 100)
    esp_netif_inherent_config_t eth_base_cfg = ESP_NETIF_INHERENT_DEFAULT_ETH();
    eth_base_cfg.route_prio = 150;
    esp_netif_config_t cfg = {
        .base = &eth_base_cfg,
        .driver = NULL,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
    };
    eth_netif_ptr = esp_netif_new(&cfg);

    // 6. Konfigurasi SPI bus (SPI3_HOST agar tidak bentrok dengan SPI Arduino di SPI2_HOST)
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num   = W5500_MOSI;
    buscfg.miso_io_num   = W5500_MISO;
    buscfg.sclk_io_num   = W5500_SCLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;

    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        Serial.printf("Gagal inisialisasi SPI Bus: %s\n", esp_err_to_name(ret));
        return;
    }

    // Di IDF 5.x yang baru, driver W5500 membuat SPI device sendiri.
    // Jadi spi_bus_add_device() TIDAK dipanggil lagi; cukup kirim konfigurasi device ke macro.
    // command_bits & address_bits diisi otomatis oleh driver.
    spi_device_interface_config_t devcfg = {};
    devcfg.mode           = 0;
    devcfg.clock_speed_hz = 10 * 1000 * 1000; // 10 MHz, stabil
    devcfg.spics_io_num   = W5500_CS;
    devcfg.queue_size     = 20;

    // 7. MAC config: pin INT harus disertakan agar driver tidak menolak
    // Beri PULLUP agar jika pin INT tidak dicolok (floating), tidak terjadi banjir interrupt palsu
    pinMode(W5500_INT, INPUT_PULLUP);

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI3_HOST, &devcfg);
    w5500_config.int_gpio_num = W5500_INT;
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    if (mac == NULL) {
        Serial.println("Gagal membuat MAC W5500 (cek wiring SPI / pin CS / RST).");
        return;
    }

    // 8. PHY config: reset sudah dilakukan manual di atas
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr       = 1;
    phy_config.reset_gpio_num = -1;
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);
    if (phy == NULL) {
        Serial.println("Gagal membuat PHY W5500.");
        return;
    }

    // 9. Install driver
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    ret = esp_eth_driver_install(&eth_config, &eth_handle);
    if (ret != ESP_OK) {
        Serial.printf("Gagal install driver W5500: %s\n", esp_err_to_name(ret));
        return;
    }

    // 10. W5500 tidak punya MAC bawaan: SET MAC SEBELUM attach dan start
    uint8_t mac_addr[6];
    esp_read_mac(mac_addr, ESP_MAC_ETH);
    ret = esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, mac_addr);
    if (ret != ESP_OK) {
        Serial.printf("Gagal set MAC W5500: %s\n", esp_err_to_name(ret));
    }

    // 11. Attach ke LwIP lalu mulai Ethernet
    esp_netif_attach(eth_netif_ptr, esp_eth_new_netif_glue(eth_handle));
    esp_eth_start(eth_handle);

    // WORKAROUND W5500: Jika kabel sudah tertancap sejak awal, W5500 mungkin sudah menarik
    // pin INT ke LOW sebelum driver siap (sehingga trigger falling-edge terlewat).
    // Kita ubah sesaat tipe interrupt menjadi LOW_LEVEL agar ISR dipaksa jalan jika pin sedang LOW,
    // lalu kembalikan ke NEGEDGE (falling edge).
    gpio_set_intr_type((gpio_num_t)W5500_INT, GPIO_INTR_LOW_LEVEL);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_intr_type((gpio_num_t)W5500_INT, GPIO_INTR_NEGEDGE);

    Serial.printf("W5500 Driver Started. MAC: %02X:%02X:%02X:%02X:%02X:%02X. Menunggu IP...\n",
                  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    // 12. Diagnostik otomatis jika 15 detik belum dapat IP
    xTaskCreate(eth_diag_task, "eth_diag", 3072, NULL, 1, NULL);
}

bool isEthernetConnected()
{
    return eth_connected;
}

bool isEthernetLinkUp()
{
    return eth_link_up;
}

IPAddress getEthernetIP()
{
    if (eth_netif_ptr) {
        esp_netif_ip_info_t ip = {};
        if (esp_netif_get_ip_info(eth_netif_ptr, &ip) == ESP_OK) {
            return IPAddress(ip.ip.addr);
        }
    }
    return IPAddress(0, 0, 0, 0);
}

String getEthernetMac()
{
    uint8_t mac_addr[6];
    esp_read_mac(mac_addr, ESP_MAC_ETH);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    return String(macStr);
}