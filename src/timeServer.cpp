#include <Arduino.h>
#include "time.h"     // library time
#include "esp_sntp.h" // library SNTP ESP32

extern String date;


// real time clock
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 25200;
const int daylightOffset_sec = 0;

// Time zone rule (optional)
const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";

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
