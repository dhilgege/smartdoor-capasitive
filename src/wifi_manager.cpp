#include <WiFi.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include "wifi_manager.h"

extern char *customHostname;
extern char *ssidwifimanager;
extern char *passwordwifimanager;

WiFiManager wm;       // Instance WiFiManager
bool portalActive = false; // Status apakah portal WiFi aktif

void setupWifi() {
  delay(1000);
  Serial.println("\nStarting");

  WiFi.mode(WIFI_STA); // Set WiFi mode ke STA (Station)

  wm.setConnectTimeout(30); // Timeout 30 detik untuk mencoba menyambung ke WiFi


  // Cobalah menyambung ke WiFi yang tersimpan tanpa membuka portal
  bool res = wm.autoConnect(ssidwifimanager, passwordwifimanager);
  if (res) {
    Serial.println("Terkoneksi ke WiFi");
  } else {
    Serial.println("Gagal menyambung, WiFi tidak tersedia.");
  }

  // Mulai task untuk memeriksa koneksi WiFi secara otomatis
  xTaskCreatePinnedToCore(
    WiFiReconnectTask,       // Fungsi task
    "WiFiReconnectTask",     // Nama task
    4096,                    // Ukuran stack
    NULL,                    // Parameter untuk task
    1,                       // Prioritas task
    NULL,                    // Handle task
    1                        // Jalankan di core 1
  );
}

void WiFiReconnectTask(void *parameter) {
  for (;;) {
    // Jika WiFi terputus dan portal tidak aktif, maka coba koneksi ulang
    if (!portalActive && WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi terputus, mencoba menyambungkan kembali...");
      WiFi.begin(); // Koneksi ulang dengan SSID dan password tersimpan
      delay(1000);  // Delay 1 detik sebelum pengecekan ulang
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
  ESP.restart();
}