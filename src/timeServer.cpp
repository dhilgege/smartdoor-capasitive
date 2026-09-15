#include <Arduino.h>
#include "time.h"     // library time
#include "esp_sntp.h" // library SNTP ESP32
#include "timeServer.h"
#include "config/config.h"

// Fungsi callback
void timeavailable(struct timeval *t);

// Inisialisasi waktu NTP
void timeSetup()
{
    sntp_set_time_sync_notification_cb(timeavailable);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}

// Fungsi untuk menampilkan waktu lokal
void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("No time available (yet)");
        timeSetup();
    }
    // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    date = "";
    // if(date){
    //     Serial.println("Date yang didapat.............: " + date);
    // }
}

// Callback ketika waktu tersedia dari NTP
void timeavailable(struct timeval *t)
{
    Serial.println("Got time adjustment from NTP!");
    printLocalTime();
}
