#include <WiFi.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include "wifi_manager.h"
#include "hardware.h"
#include "config/config.h"
#include "ethernet_manager.h"

WiFiManager wm;       // Instance WiFiManager
bool portalActive = false; // Status apakah portal WiFi aktif

void setupWifi() {
  delay(1000);
  Serial.println("\n[WiFi] Memulai setup WiFi...");

  WiFi.mode(WIFI_STA); // Set WiFi mode ke STA (Station)

  // Set timeout portal 30 detik agar tidak memblokir main loop selamanya
  wm.setConfigPortalTimeout(30);

  // Cobalah menyambung ke WiFi yang tersimpan tanpa membuka portal
  bool res = wm.autoConnect(ssidwifimanager, passwordwifimanager);
  if (res) {
    Serial.println("[WiFi] Terkoneksi ke WiFi");
    TurnOFFLed("RED");
    TurnOnLed("BLUE");
  } else {
    Serial.println("[WiFi] Gagal menyambung, WiFi tidak tersedia.");
  }
}

void WiFiReconnectTask(void *parameter) {
  int ethDownCycles = 0;
  bool wifiActiveState = false;

  for (;;) {
    if (isEthernetConnected()) {
      ethDownCycles = 0;
      // Jika Ethernet aktif tetapi radio WiFi masih menyala, matikan WiFi
      if (WiFi.getMode() != WIFI_OFF) {
        Serial.println("[Failover] Ethernet (LAN) aktif. Mematikan radio WiFi cadangan.");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        wifiActiveState = false;
        TurnOFFLed("RED");
      }
    } else {
      ethDownCycles++;
      // Jika Ethernet terputus selama 2 siklus (>= 10 detik), aktifkan WiFi cadangan
      if (ethDownCycles >= 2 && !portalActive) {
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("[Failover] Ethernet (LAN) terputus! Mengaktifkan WiFi cadangan...");
          WiFi.mode(WIFI_STA);
          WiFi.begin();
        } else if (!wifiActiveState) {
          wifiActiveState = true;
          Serial.println("========================================");
          Serial.println("[Failover] WiFi cadangan AKTIF & TERHUBUNG!");
          Serial.println("[Failover] SSID : " + String(WiFi.SSID()));
          Serial.println("[Failover] IP   : " + WiFi.localIP().toString());
          Serial.println("========================================");
          TurnOFFLed("RED");
        }
      }
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Cek ulang setiap 5 detik
  }
}

void openWebPortal() {
  if (!portalActive) {
    portalActive = true;
    Serial.println("Membuka portal WiFiManager...");

    // Hentikan task WiFi reconnect agar tidak mengganggu portal
    wm.setConfigPortalBlocking(false);   // Atur portal non-blocking
    if (!wm.startConfigPortal("OnDemandAP", "password")) {
      Serial.println("Timeout portal konfigurasi");
    } else {
      Serial.println("WiFi dikonfigurasi ulang.");
    }
    
    portalActive = false;  // Set portal tidak aktif setelah selesai
    ESP.restart();         // Restart untuk memulai ulang koneksi baru
  } else {
    Serial.println("Portal sudah aktif.");
  }
}


void resetEpproomWifi() {
  wm.resetSettings();
  // ESP.restart();
}