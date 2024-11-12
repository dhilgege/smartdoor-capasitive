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

void loginDevice(String macAddress)
{
  while (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = String(ApiLogin) + "/" + macAddress;
    http.begin(url);
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");

    // Buat payload JSON jika diperlukan
    String jsonPayload = "{\"mac_address\":\"" + macAddress + "\"}";

    int httpResponseCode = http.POST(jsonPayload);
    Serial.println("mengirim ke: " + url);

    if (httpResponseCode == HTTP_CODE_OK)
    {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);

      // Bersihkan tanda kutip di awal dan akhir
      response.replace("\"", "");

      response.replace("Bearer ", "");

      AuthToken = response;

      Serial.println("Cleaned AuthToken: " + AuthToken);

      break; // Keluar dari loop jika berhasil login
    }
    else
    {
      Serial.print("Error Code: ");
      Serial.println(httpResponseCode);
      Serial.println("Login Gagal, coba lagi...");
    }
    http.end();
    delay(5000);
  }
}

void detectPinSend()
{

  // Cek status detectPin
  if (digitalRead(detectPin) == LOW)
  {

    HTTPClient http;
    String url = String(ApiUpdateStatusDevice);
    http.begin(url); // Memulai HTTP request ke URL API
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + AuthToken);

    int detectPinState = digitalRead(detectPin);
    String statusDevice;

    // Buat payload JSON
    String macAddress = WiFi.macAddress();
    String jsonPayload = "{\"mac_address\":\"" + macAddress + "\", \"status\":\"" + 2 + "\"}";

    int httpResponseCode = http.POST(jsonPayload);
    Serial.println("Mengirim ke: " + url);
    Serial.println("Payload: " + jsonPayload);

    if (httpResponseCode == HTTP_CODE_OK)
    {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
    }
    else
    {
      Serial.print("Error Code: ");
      Serial.println(httpResponseCode);
      Serial.println("Gagal mengirim data, coba lagi...");
    }
    http.end();
    delay(5000);
  }
}

// versi perulangan
void sendHistoryRfid(String rfid)
{

  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = String(ApiSendHistory);
    http.begin(url);
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + AuthToken);


    // Buat payload JSON jika diperlukan
    String jsonPayload = "{\"uid\":\"" + rfid + "\", \"mac_address\":\"" + macAddress + "\"}";

    int httpResponseCode = http.POST(jsonPayload);
    Serial.println("mengirim ke: " + url);

    if (httpResponseCode == HTTP_CODE_OK)
    {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);

    // return true;
    }
    else
    {
      Serial.print("Error Code: ");
      Serial.println(httpResponseCode);
      Serial.println("Kirim akses gagal...");
    // return false;
    }
    http.end();
    delay(5000);
  }

}