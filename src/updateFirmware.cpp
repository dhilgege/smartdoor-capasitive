#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "updateFirmware.h"
#include "config/config.h"
#include <config/apidata.h>

void updateFirmware()
{
    Serial.println("===== START OTA UPDATE =====");

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi tidak terhubung!");
        return;
    }

    WiFiClient client;
    HTTPClient http;

    String updateUrl = String(ServerName) + String(ApiFirmwareUpdate);

    Serial.println("Requesting firmware from:");
    Serial.println(updateUrl);

    http.begin(client, updateUrl);

    // Kirim MAC address agar Laravel bisa validasi device
    http.addHeader("X-Device-Mac", macAddress);
    http.addHeader("Authorization", "Bearer " + AuthToken);

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("HTTP Error: %d\n", httpCode);
        http.end();
        Serial.println("OTA gagal, restart...");
        delay(2000);
        ESP.restart();
        return;
    }

    int contentLength = http.getSize();

    if (contentLength <= 0)
    {
        Serial.println("Content-Length invalid");
        http.end();
        ESP.restart();
        return;
    }

    bool canBegin = Update.begin(contentLength);

    if (!canBegin)
    {
        Serial.println("Not enough space for OTA");
        http.end();
        ESP.restart();
        return;
    }

    Serial.println("Downloading firmware...");

    WiFiClient *stream = http.getStreamPtr();
    size_t written = Update.writeStream(*stream);

    if (written != contentLength)
    {
        Serial.printf("Written only %d/%d bytes\n", written, contentLength);
    }

    if (!Update.end())
    {
        Serial.printf("Update error: %d\n", Update.getError());
        http.end();
        Serial.println("OTA gagal, restart...");
        delay(2000);
        ESP.restart();
        return;
    }

    if (!Update.isFinished())
    {
        Serial.println("Update not finished?");
        http.end();
        ESP.restart();
        return;
    }

    Serial.println("OTA SUCCESS. Restarting...");
    http.end();
    delay(2000);
    ESP.restart();
}