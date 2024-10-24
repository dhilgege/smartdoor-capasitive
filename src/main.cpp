#include <Arduino.h>
#include <config/apidata.h>
#include <WiFi.h>

#include "wifi_manager.h"
#include "rfid.h"
#include "hardware.h"
#include "config/config.h"
#include "hitApi.h"
#include "fungsi_keypad.h"


// librarinya
#include "time.h"     //time
#include "esp_sntp.h" //time



// real time clock
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 25200;
const int daylightOffset_sec = 0;

const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3"; // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void midnightTask();

void setup()
{
  Serial.begin(115200);                        // Initialize serial communications with the PC
  // Serial1.begin(9600, SERIAL_8N1, RXp2, TXp2); // Initialize serial communications with the PC

  pinSetup();
  setupRfid();
  setupWiFi();
  checkConnection();

  // Dapatkan MAC address ESP32
  macAddress = WiFi.macAddress();

  // time
  //  set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

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

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command == "restartsekarang") {
      Serial.println("Restarting...");
      ESP.restart();
    }
  }

  // detectPinSend();

  Serial.println("AuthToken: " + AuthToken);
  printLocalTime(); // it will take some time to sync time :)

  // Pemeriksaan status device
  GetStatusDevice();

  // tugas tengah malem
  midnightTask();

  // hardResetSettings();


  if (status_device == "0")
  {
    ActivateDoorTimer(3000);
  }
  ActivateDoor(false);
  Serial.println("pintu ditutup.HIGH");

  checkConnection();

  cekRfid();
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

