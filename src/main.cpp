// include lib dari luar
#include <Arduino.h>
#include <config/apidata.h>
#include <WiFi.h>

// #include "multitask/wifi_task.h"
#include "wifi_manager.h"
// #include "rfid.h"
#include "rfid125.h"
#include "hardware.h"
#include "config/config.h"
#include "hitApi.h"
#include "fungsi_keypad.h"
#include "timeServer.h"
#include "handleSerialInput.h"
#include "multitask/keypad_task.h"
#include "multitask/api_task.h"

void midnightTask();

TaskHandle_t WiFiTaskHandle;
TaskHandle_t getStatusTaskHandle;
TaskHandle_t KeypadTaskHandle;

void setup()
{
    Serial.begin(115200);
    ActivateDoor(false);

    setupWifi();

    // xTaskCreatePinnedToCore(
    //     WiFiTask,
    //     "Check Connection Task",
    //     3072,
    //     NULL,
    //     2,
    //     &WiFiTaskHandle,
    //     0 // Core 0
    // );

    xTaskCreatePinnedToCore(
        getStatusTask,
        "Status Task",
        4096,
        NULL,
        1,
        &getStatusTaskHandle,
        0 // Core 1
    );

    xTaskCreatePinnedToCore(
        KeypadTask,
        "Handle Keypad Input",
        8192,
        NULL,
        3,
        &KeypadTaskHandle,
        1 // Core 1
    );

    // xTaskCreatePinnedToCore(
    //     KeypadTask,
    //     "Handle Keypad Input",
    //     2048,
    //     NULL,
    //     3,
    //     &KeypadTaskHandle,
    //     1 // Core 1
    // );

    pinSetup();
    rfid125Setup();
    // setupRfid();
    // checkConnection();

    // Dapatkan MAC address ESP32
    macAddress = WiFi.macAddress();

    // time
    timeSetup();

    Serial.println("Nama WiFi: " + String(WiFi.SSID()));
    Serial.println("IP Address sekarang: " + WiFi.localIP().toString());
    Serial.println("MAC Address: " + macAddress);
    Serial.println("Hostname ESP32: " + String(WiFi.getHostname()));

    loginDevice(macAddress);

    // Tampilkan token yang didapat
    getAllRfid(); // Download semua data RFID setelah login
    getNewPin();  // ambil pin baru
}

void loop()
{

    handleKeypad(); // harusnya dihapus

    handleSerialInput();

    // Serial.println("AuthToken: " + AuthToken);
    printLocalTime(); // it will take some time to sync time :)

    // Pemeriksaan status device
    // GetStatusDevice();

    // tugas tengah malem
    midnightTask();

    // hardResetSettings();

    if (status_device == "0")
    {
        ActivateDoor(true);
    }
    else if (status_device == "1")
    {
        ActivateDoor(false);
    }
    //   ActivateDoor(false);
    // Serial.println("pintu ditutup.HIGH");

    // checkConnection();

    readRfid125();
    //   checkConnection();
}

int lastUpdateDay = -1;        // Hari terakhir update
bool updateInProgress = false; // Cegah duplikasi pengambilan data

void midnightTask()
{
    // Dapatkan waktu lokal
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Waktu lokal belum tersedia");
        return;
    }

    // Cek apakah hari telah berganti dan sudah masuk jam 00:00
    if (timeinfo.tm_hour == 13 && timeinfo.tm_min == 26)
    {
        // Cek apakah pengambilan data sudah dilakukan hari ini
        if (lastUpdateDay != timeinfo.tm_mday && !updateInProgress)
        {
            updateInProgress = true; // Tandai bahwa proses update sedang berlangsung
            Serial.println("Memulai pengambilan data RFID dan PIN baru...");

            // Ulangi pengambilan data hingga berhasil
            while (!getAllRfid() || !getNewPin())
            {
                Serial.println("Pengambilan data gagal, mencoba ulang...");
                delay(5000); // Tunggu sebelum mencoba lagi
            }

            // Jika berhasil, simpan hari terakhir update
            lastUpdateDay = timeinfo.tm_mday;
            Serial.println("Pengambilan data berhasil!");

            updateInProgress = false; // Reset status proses update
        }
    }
}
