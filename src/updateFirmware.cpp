#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "updateFirmware.h"
#include "config/config.h"
#include "config/apidata.h"
#include "ethernet_manager.h"

void updateFirmware()
{
    Serial.println("===== START OTA UPDATE =====");

    if (WiFi.status() != WL_CONNECTED && !isEthernetConnected())
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
    http.setTimeout(15000);
    http.addHeader("Accept", "application/octet-stream, application/json");
    http.addHeader("X-Requested-With", "XMLHttpRequest");

    // Kirim MAC address agar Laravel bisa validasi device
    http.addHeader("X-Device-Mac", macAddress);
    http.addHeader("Authorization", "Bearer " + AuthToken);
    http.addHeader("Connection", "close");

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("HTTP Error: %d\n", httpCode);
        String errResp = http.getString();
        errResp.trim();
        if (errResp.startsWith("<"))
        {
            int titleStart = errResp.indexOf("<title>");
            int titleEnd = errResp.indexOf("</title>");
            if (titleStart != -1 && titleEnd != -1) {
                Serial.println("[HTML Error] " + errResp.substring(titleStart + 7, titleEnd));
            }
        }
        else if (errResp.length() > 0)
        {
            Serial.println("Respon Error: " + errResp);
        }
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