// include lib dari luar
#include <Arduino.h>
#include <config/apidata.h>
#include <WiFi.h>

#include "multitask/wifi_task.h"
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
  Serial.begin(115200); // Initialize serial communications with the PC
  // Serial1.begin(9600, SERIAL_8N1, RXp2, TXp2); // Initialize serial communications with the PC

  setupWiFi();

xTaskCreatePinnedToCore(
    WiFiTask,
    "Check Connection Task",
    3072,
    NULL,
    2,
    &WiFiTaskHandle,
    0 // Core 0
);

xTaskCreatePinnedToCore(
    getStatusTask,
    "Status Task",
    3072,
    NULL,
    1,
    &getStatusTaskHandle,
    0 // Core 1
);

xTaskCreatePinnedToCore(
    KeypadTask,
    "Handle Keypad Input",
    2048,
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

  handleKeypad();

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
    ActivateDoorTimer(3000);
  }
  ActivateDoor(false);
  // Serial.println("pintu ditutup.HIGH");

  // checkConnection();

  readRfid125();
}
















void midnightTask()
{
  // Dapatkan waktu lokal
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Waktu lokal belum tersedia");
    return;
  }

  // Cek apakah sekarang sudah jam 00:00
  if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0)
  {

    // proses pengambilan rfid tengah malam
    getAllRfid();

    // proses ambil/cek pin baru
    getNewPin();
  }
}
