#include "wifi_manager.h"
#include <HTTPClient.h>
#include <config/apidata.h>

extern String AuthToken;
extern String macAddress;
extern String status_device;
extern int rfidCount;
extern String rfids[];
extern String StoredDoorPin;
extern int detectPin;

void GetStatusDevice()
{
    if (WiFi.status() == WL_CONNECTED)
    {

        HTTPClient http;
        // Buat URL dengan MAC address
        String url = String(ApiStatusDevice) + "/" + macAddress;
        Serial.println("URL: " + url); // Cetak URL untuk debugging

        http.begin(url);
        http.addHeader("Authorization", "Bearer " + AuthToken);

        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK)
        {
            String respon = http.getString();
            Serial.println("Status device: " + respon);

            // Ambil status device
            status_device = respon; // Angka 1 atau 0

            Serial.println("Status stored: " + status_device);
        }
        else
        {
            Serial.print("Error Code: ");
            Serial.println(httpCode);
        }
        http.end();
    }
    delay(1000);
}

void getAllRfid()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String url = String(ApiAllRfid) + "/" + macAddress;
        http.begin(url);
        Serial.println("sending to" + url);
        http.addHeader("Authorization", "Bearer " + AuthToken);

        int httpResponseCode = http.GET();
        if (httpResponseCode == HTTP_CODE_OK)
        {
            String response = http.getString();
            Serial.println("RFID Response: " + response);

            response.replace("[", "");  // Remove '['
            response.replace("]", "");  // Remove ']'
            response.replace("\"", ""); // Remove '\"'
            Serial.println("rfid Clear Response" + response);

            int startIndex = 0;
            int endIndex = response.indexOf(',');

            while (endIndex != -1)
            {
                rfids[rfidCount] = response.substring(startIndex, endIndex);
                rfidCount++;
                startIndex = endIndex + 1;
                endIndex = response.indexOf(',', startIndex);
            }
            rfids[rfidCount] = response.substring(startIndex); // Last RFID
            rfidCount++;

            Serial.print("Jumlah RFID yang di-download: ");
            Serial.println(rfidCount);
        }
        else
        {
            Serial.print("Error Code: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    }
    else
    {
        Serial.println("WiFi Disconnected");
        checkConnection();
    }
}

void getNewPin()
{
    if (WiFi.status() == WL_CONNECTED)
    {

        HTTPClient http;
        String url = String(ApiNewPin) + "/" + macAddress;
        Serial.println("URL: " + url); // debugging

        http.begin(url);
        http.addHeader("Authorization", "Bearer " + AuthToken);

        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK)
        {
            String respon = http.getString();
            respon.replace("[", "");  // hapus  '['
            respon.replace("]", "");  // hapus  ']'
            respon.replace("\"", ""); // hapus  '"'

            Serial.println("Pin yang diambil: " + respon);

            // Ambil status device
            StoredDoorPin = respon;

            Serial.println("Pin stored: " + StoredDoorPin);
            Serial1.println("PWD:" + StoredDoorPin); // Buka kunci
            Serial.println("berhasil mengirim pin baru " + StoredDoorPin);
            delay(1000);
        }
        else
        {
            Serial.print("Error Code: ");
            Serial.println(httpCode);
        }
        http.end();
    }
    delay(1000);
}
