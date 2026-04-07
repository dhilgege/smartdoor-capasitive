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
#include "updateFirmware.h"

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
    Serial.begin(115200);
    ActivateDoor(false);
    getOneTimeDone = false;

    // Mulai task untuk memeriksa koneksi WiFi secara otomatis
    xTaskCreatePinnedToCore(
        WiFiReconnectTask,   // Fungsi task
        "WiFiReconnectTask", // Nama task
        4096,                // Ukuran stack
        NULL,                // Parameter untuk task
        1,                   // Prioritas task
        NULL,                // Handle task
        0                    // Jalankan di core 1
    );

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

    pinSetup();
    rfid125Setup();
    setupWifi();
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

    if (isOtaMode) { delay(1000); return;} // pengecekan OTA harus diatas loop main(prioritas tertinggi)

    getOneTime();
    handleSerialInput();
    printLocalTime(); // it will take some time to sync time :)
    midnightTask();
    readRfid125();
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