#include <WiFi.h>
#include "wifi_manager.h"
#include <HTTPClient.h>
#include "hitApi.h"
#include "hardware.h"
#include "config/config.h"
#include "otaHandleMain.h"
#include "ethernet_manager.h"


void getStatusAndPinDevice()
{
    if (WiFi.status() == WL_CONNECTED || isEthernetConnected())
    {
        if (AuthToken.length() > 0 && macAddress.length() > 0)
        {
            HTTPClient http;
            String url = String(ServerName) + String(ApiStatusAndPinDevice) + "/" + macAddress;
            Serial.println("URL: " + url);

            http.begin(url);
            http.setTimeout(2500);
            http.addHeader("Accept", "application/json");
            http.addHeader("X-Requested-With", "XMLHttpRequest");
            http.addHeader("Authorization", "Bearer " + AuthToken);
            http.addHeader("Connection", "close");

            int httpCode = http.GET();

            if (httpCode == HTTP_CODE_OK)
            {
                String respon = http.getString();
                respon.trim();

                // Cegah parsing jika respon tidak sengaja mengembalikan halaman HTML
                if (respon.startsWith("<") || respon.indexOf("<html") != -1)
                {
                    Serial.println("[API] Peringatan: Respon berformat HTML, bukan JSON.");
                    http.end();
                    return;
                }

                Serial.println("Respon: " + respon);

                // Ambil pin_device
                int pinIndex = respon.indexOf("\"pin_device\":");
                if (pinIndex != -1)
                {
                    pinIndex += 13;
                    int pinEnd = respon.indexOf(",", pinIndex);
                    if (pinEnd == -1) pinEnd = respon.indexOf("}", pinIndex); // fallback jika terakhir
                    String pinStr = respon.substring(pinIndex, pinEnd);
                    pinStr.trim();
                    StoredDoorPin = pinStr.toInt();
                }

                // Ambil status
                int statusIndex = respon.indexOf("\"status\":");
                if (statusIndex != -1)
                {
                    statusIndex += 9;
                    int statusEnd = respon.indexOf(",", statusIndex);
                    if (statusEnd == -1) statusEnd = respon.indexOf("}", statusIndex);
                    String statusStr = respon.substring(statusIndex, statusEnd);
                    statusStr.trim();
                    status_device = statusStr;
                }

                Serial.print("Pin Device: ");
                Serial.println(StoredDoorPin);
                Serial.print("Status stored: ");
                Serial.println(status_device);
            }
            else if (httpCode == 302)
            {
                loginDevice(macAddress);
            }
            else
            {
                Serial.print("Error Code: ");
                Serial.println(httpCode);
                String errResp = http.getString();
                errResp.trim();
                if (errResp.startsWith("<"))
                {
                    int titleStart = errResp.indexOf("<title>");
                    int titleEnd = errResp.indexOf("</title>");
                    if (titleStart != -1 && titleEnd != -1) {
                        Serial.println("[HTML Error] " + errResp.substring(titleStart + 7, titleEnd));
                    } else {
                        Serial.println("[HTML Error] Server mengembalikan halaman HTML.");
                    }
                }
                else if (errResp.length() > 0)
                {
                    Serial.println("Respon: " + errResp);
                }
            }

            http.end();
        }
    }
}


bool getAllRfid()
{
    if (WiFi.status() == WL_CONNECTED || isEthernetConnected())
    {
        HTTPClient http;
        String url = String(ServerName) + String(ApiAllRfid) + "/" + macAddress;
        playDownloadTone();
        http.begin(url);
        http.setTimeout(5000);
        http.addHeader("Accept", "application/json");
        http.addHeader("X-Requested-With", "XMLHttpRequest");
        http.addHeader("Authorization", "Bearer " + AuthToken);
        http.addHeader("Connection", "close");
        Serial.println("sending to " + url);

        int httpResponseCode = http.GET();
        if (httpResponseCode == HTTP_CODE_OK)
        {
            String response = http.getString();
            response.trim();

            if (response.startsWith("<") || !response.startsWith("["))
            {
                Serial.println("[API] Respon RFID bukan array JSON valid!");
                playErrorTone();
                http.end();
                return false;
            }

            Serial.println("RFID Response: " + response);

            response.replace("[", "");  // Hapus '['
            response.replace("]", "");  // Hapus ']'
            response.replace("\"", ""); // Hapus '"'

            int startIndex = 0;
            int endIndex = response.indexOf(',');

            rfidCount = 0;
            while (endIndex != -1)
            {
                rfids[rfidCount] = response.substring(startIndex, endIndex);
                rfidCount++;
                startIndex = endIndex + 1;
                endIndex = response.indexOf(',', startIndex);
            }
            rfids[rfidCount] = response.substring(startIndex); // RFID terakhir
            rfidCount++;

            Serial.print("Jumlah RFID yang di-download: ");
            Serial.println(rfidCount);
            playSuccessTone();
            http.end();
            return true;
        }
        else if (httpResponseCode == 302)
        {
            loginDevice(macAddress);
            http.end();
            return false;
        }
        else
        {
            Serial.print("Error Code: ");
            Serial.println(httpResponseCode);
            String errResp = http.getString();
            errResp.trim();
            if (errResp.startsWith("<"))
            {
                int titleStart = errResp.indexOf("<title>");
                int titleEnd = errResp.indexOf("</title>");
                if (titleStart != -1 && titleEnd != -1) {
                    Serial.println("[HTML Error] " + errResp.substring(titleStart + 7, titleEnd));
                } else {
                    Serial.println("[HTML Error] Server mengembalikan halaman HTML.");
                }
            }
            else if (errResp.length() > 0)
            {
                Serial.println("Respon: " + errResp);
            }
            playErrorTone();
            http.end();
            return false;
        }
    }
    else
    {
        Serial.println("GetRfid Wifi Disconect");
        // checkConnection();
        return false;
    }

    return false; // Jaga-jaga: semua jalur eksekusi pasti mengembalikan nilai
}

bool getNewPin()
{
    // langsung return true karena pin device diambil dari getStatusAndPinDevice
    return true;
}

void getStatusTask(void *pvParameters)
{
    for (;;)
    {
        if (isOtaMode)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        if (WiFi.status() == WL_CONNECTED || isEthernetConnected())
        {
            if (AuthToken.length() > 0 && macAddress.length() > 0)
            {
                getStatusAndPinDevice();
            }
        }
        else
        {
            Serial.println("Jaringan terputus, gagal ambil status");
        }

        if (status_device == "3")
        {
            enterOtaMode();
        }

        if (status_device == "0" && !doorTimerActive)
        {
            ActivateDoorTimer(3000);
            Serial.println("Door open by API (timer)");
            status_device = "-1";
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS); // Polling 2s untuk mencegah kehabisan TCP socket PCB
    }
}

void GET()
{
    String response;
    if (WiFi.status() == WL_CONNECTED || isEthernetConnected())
    {
        if (AuthToken.length() > 0)
        {
            HTTPClient http;
            String url = "http://localhost:8000/api/getdata" + macAddress;
            Serial.println("URL: " + url); // Cetak URL untuk debugging

            http.begin(url);
            http.addHeader("Accept", "application/json");
            http.addHeader("X-Requested-With", "XMLHttpRequest");
            http.addHeader("Authorization", "Bearer " + AuthToken);
            http.addHeader("Connection", "close");

            int httpCode = http.GET(); // ambil respon kode: 200, 500, 404 dll

            if (httpCode == HTTP_CODE_OK)
            {
                String respon = http.getString(); // ambil data yang didapat
                Serial.println(respon);
                response = respon;
            }
            else if (httpCode == 302)
            {
                loginDevice(macAddress);
            }
            else
            {
                Serial.print("Error Code: ");
                Serial.println(httpCode);
            }
            http.end();
        }
    }
}