// Aktifkan ini untuk debug
#define DEBUG_MODE
// Aktifkan ini untuk debug

#ifdef DEBUG_MODE
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif



// include lib dari luar
#include <Arduino.h>
#include <config/apidata.h>
#include <WiFi.h>
#include <esp_mac.h>

// #include "multitask/wifi_task.h"
#include "wifi_manager.h"
#include "rfid.h"
#include "hardware.h"
#include "config/config.h"
#include "hitApi.h"
#include "fungsi_keypad.h"
#include "timeServer.h"
#include "handleSerialInput.h"
#include "multitask/keypad_task.h"
#include "multitask/api_task.h"
#include "updateFirmware.h"
#include "ethernet_manager.h"

void midnightTask();
void getOneTime();
void enterOtaMode();

TaskHandle_t WiFiTaskHandle;
TaskHandle_t getStatusTaskHandle;
TaskHandle_t KeypadTaskHandle;

bool getOneTimeDone;
// extern unsigned long doorTimerStart;
// extern int doorTimerDuration;
// extern bool doorTimerActive;

void setup()
{
    Serial.begin(9600);
    pinSetup();   // HARUS di atas xTaskCreate agar BUZZER_PIN sudah siap
    ActivateDoor(false);
    getOneTimeDone = false;

    // Test buzzer saat startup
    tone(BUZZER_PIN, 1000, 150);
    delay(200);
    tone(BUZZER_PIN, 1500, 150);
    delay(200);

    // Inisialisasi MAC address lebih awal agar tidak kosong.
    // Dibaca dari eFuse: WiFi.macAddress() masih 00:00:00:00:00:00 sebelum WiFi diinisialisasi.
    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char macBuf[18];
    snprintf(macBuf, sizeof(macBuf), "%02X:%02X:%02X:%02X:%02X:%02X",
             baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    macAddress = String(macBuf);

    rfidSetup();
    ethernetSetup();

    // Beri waktu sejenak (hingga 4 detik) untuk memastikan kabel LAN menancap (Link Up)
    Serial.println("Mengecek kabel Ethernet (LAN)...");
    unsigned long ethWaitStart = millis();
    while (!isEthernetLinkUp() && (millis() - ethWaitStart < 4000)) {
        delay(200);
    }

    if (isEthernetLinkUp()) {
        Serial.println("[NET] Kabel LAN terdeteksi (Link Up)! Menunggu DHCP (hingga 15 detik)...");
        unsigned long dhcpWaitStart = millis();
        while (!isEthernetConnected() && (millis() - dhcpWaitStart < 15000)) {
            delay(200);
        }
    }

    if (isEthernetConnected()) {
        Serial.println("========================================");
        Serial.println("[NET] Ethernet (LAN) BERHASIL terhubung!");
        Serial.println("[NET] IP Address : " + getEthernetIP().toString());
        Serial.println("[NET] MAC Ethernet : " + getEthernetMac());
        Serial.println("[NET] MAC WiFi     : " + macAddress);
        Serial.println("========================================");
        // Matikan radio WiFi untuk kestabilan daya (mengurangi beban 3.3V) & mencegah konflik rute
        WiFi.mode(WIFI_OFF);
    } else {
        Serial.println("[NET] Ethernet belum terhubung, mencoba koneksi WiFi fallback...");
        setupWifi();
        Serial.println("Nama WiFi: " + String(WiFi.SSID()));
        Serial.println("IP WiFi: " + WiFi.localIP().toString());
        Serial.println("MAC WiFi: " + macAddress);
    }

    // Mulai task Keypad
    xTaskCreatePinnedToCore(
        KeypadTask,
        "Handle Keypad Input",
        8192,
        NULL,
        3,
        &KeypadTaskHandle,
        1 // Core 1
    );

    // Mulai task WiFi Reconnect (failover jika Ethernet mati)
    xTaskCreatePinnedToCore(
        WiFiReconnectTask,   // Fungsi task
        "WiFiReconnectTask", // Nama task
        4096,                // Ukuran stack
        NULL,                // Parameter untuk task
        1,                   // Prioritas task
        &WiFiTaskHandle,     // Handle task
        0                    // Core 0
    );

    // Mulai task Status Polling
    xTaskCreatePinnedToCore(
        getStatusTask,
        "Status Task",
        4096,
        NULL,
        1,
        &getStatusTaskHandle,
        0 // Core 0
    );

    // time
    timeSetup();

    loginDevice(macAddress);

    // Tampilkan token yang didapat
    getAllRfid(); // Download semua data RFID setelah login
    getNewPin();  // ambil pin baru
}

void loop()
{

    if (isOtaMode) { delay(1000); return;} // pengecekan OTA harus diatas loop main(prioritas tertinggi)

    getOneTime();
    handleSerialInput();
    printLocalTime(); // it will take some time to sync time :)
    midnightTask();
    readRfid();
}

int lastUpdateDay = -1;        // Hari terakhir update
bool updateInProgress = false; // Cegah duplikasi pengambilan data

void midnightTask()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Waktu lokal belum tersedia");
        return;
    }

    // Jam 00:00 reset
    if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0)
    {
        if (lastUpdateDay != timeinfo.tm_mday)
        {
            lastUpdateDay = timeinfo.tm_mday;
            getOneTimeDone = false;
            Serial.println("Reset getOneTimeDone ke false untuk eksekusi ulang tengah malam 🌙");
        }
    }
}

void getOneTime()
{
    if (getOneTimeDone == false)
    {
        Serial.println("Memulai getOneTime...");
        if (getNewPin())
        {
            if (getAllRfid())
            {
                getOneTimeDone = true;
                Serial.println("getOneTime selesai dan sukses ✅");
            }
            else
            {
                Serial.println("Gagal getAllRfid ❌");
            }
        }
        else
        {
            Serial.println("Gagal getNewPin ❌");
        }
    }
}

void enterOtaMode()
{
    if (isOtaMode){
        Serial.println("===== loop if is ota mode =====");
        return;
    }

    Serial.println("===== ENTER OTA MODE =====");

    isOtaMode = true;

    ActivateDoor(false);

    // suspend task lain (opsional karena sudah ada flag guard)
    // if (KeypadTaskHandle != NULL)
    //     vTaskSuspend(KeypadTaskHandle);

    // if (getStatusTaskHandle != NULL)
    //     vTaskSuspend(getStatusTaskHandle);

    delay(1500);

    updateFirmware();
}