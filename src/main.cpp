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
void getOneTime();

TaskHandle_t WiFiTaskHandle;
TaskHandle_t getStatusTaskHandle;
TaskHandle_t KeypadTaskHandle;

bool getOneTimeDone;
extern unsigned long doorTimerStart;
extern int doorTimerDuration;
extern bool doorTimerActive;

void setup()
{
    Serial.begin(115200);
    ActivateDoor(false);
    getOneTimeDone = false;

    // setupWifi();

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
    if (doorTimerActive && millis() - doorTimerStart >= doorTimerDuration)
    {
        ActivateDoor(false); // Matikan pintu setelah waktu habis
        Serial.println("Door Closed");
        doorTimerActive = false;
    }
    if (status_device == "0" && !doorTimerActive)
    {
        ActivateDoorTimer(3000); // buka 3 detik by API
        Serial.println("Door open by API (timer)");
        status_device = "-1"; // supaya gak terus-terusan aktif
    }

    getOneTime();
    // handleKeypad(); // harusnya dihapus
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

// void midnightTask()
// {
//     // Dapatkan waktu lokal
//     struct tm timeinfo;
//     if (!getLocalTime(&timeinfo))
//     {
//         Serial.println("Waktu lokal belum tersedia");
//         return;
//     }

//     // Cek apakah hari telah berganti dan sudah masuk jam 00:00
//     if (timeinfo.tm_hour == 13 && timeinfo.tm_min == 26)
//     {
//         // Cek apakah pengambilan data sudah dilakukan hari ini
//         if (lastUpdateDay != timeinfo.tm_mday && !updateInProgress)
//         {
//             updateInProgress = true; // Tandai bahwa proses update sedang berlangsung
//             Serial.println("Memulai pengambilan data RFID dan PIN baru...");

//             // Ulangi pengambilan data hingga berhasil
//             while (!getAllRfid() || !getNewPin())
//             {
//                 Serial.println("Pengambilan data gagal, mencoba ulang...");
//                 delay(5000); // Tunggu sebelum mencoba lagi
//             }

//             // Jika berhasil, simpan hari terakhir update
//             lastUpdateDay = timeinfo.tm_mday;
//             Serial.println("Pengambilan data berhasil!");

//             updateInProgress = false; // Reset status proses update
//         }
//     }
// }
