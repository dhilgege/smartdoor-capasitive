#include <WiFi.h>
#include "wifi_manager.h"
#include <HTTPClient.h>
#include "config/config.h"
#include "hitApi.h"
#include "ethernet_manager.h"
#include "hardware.h"


void loginDevice(String macAddress)
{
  while (WiFi.status() == WL_CONNECTED || isEthernetConnected())
  {
    HTTPClient http;
    String url = String(ServerName) + String(ApiLogin) + "/" + macAddress;
    http.begin(url);
    http.setTimeout(5000);
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Requested-With", "XMLHttpRequest");
    http.addHeader("Connection", "close");

    // Buat payload JSON
    String jsonPayload = "{\"mac_address\":\"" + macAddress + "\"}";

    int httpResponseCode = http.POST(jsonPayload);
    Serial.println("mengirim ke: " + url);

    if (httpResponseCode == HTTP_CODE_OK)
    {
      String response = http.getString();
      response.trim();

      // Pastikan bukan dokumen HTML
      if (response.startsWith("<"))
      {
        Serial.println("[API] Server mengembalikan halaman HTML, bukan token JSON.");
        setErrorState(true);
        http.end();
        delay(5000);
        continue;
      }

      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);

      // Bersihkan tanda kutip di awal dan akhir
      response.replace("\"", "");
      response.replace("Bearer ", "");

      AuthToken = response;

      Serial.println("Cleaned AuthToken: " + AuthToken);
      setErrorState(false);

      http.end();
      break; // Keluar dari loop jika berhasil login
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
          Serial.println("[HTML Error] Server mengembalikan halaman HTML (Status " + String(httpResponseCode) + ").");
        }
      }
      else if (errResp.length() > 0)
      {
        int msgKey = errResp.indexOf("\"message\":");
        if (msgKey != -1)
        {
          int msgStart = errResp.indexOf("\"", msgKey + 10);
          int msgEnd = (msgStart != -1) ? errResp.indexOf("\"", msgStart + 1) : -1;
          if (msgStart != -1 && msgEnd != -1)
          {
            String serverMsg = errResp.substring(msgStart + 1, msgEnd);
            Serial.println("Pesan Server: \"" + serverMsg + "\"");
            if (serverMsg.indexOf("createToken() on null") != -1)
            {
              Serial.println(">> PERHATIAN: Device dengan MAC ini (" + macAddress + ") BELUM TERDAFTAR di database Smart Building!");
              Serial.println(">> Silakan login ke web admin http://192.168.200.20 dan daftarkan MAC address perangkat ini.");
            }
          }
          else
          {
            Serial.println("Server Response: " + errResp);
          }
        }
        else
        {
          Serial.println("Server Response: " + errResp);
        }
      }
      Serial.println("Login Gagal, coba lagi...");
      setErrorState(true);
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
    String url = String(ServerName) + String(ApiUpdateStatusDevice);
    http.begin(url); // Memulai HTTP request ke URL API
    http.setTimeout(5000);
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Requested-With", "XMLHttpRequest");
    http.addHeader("Authorization", "Bearer " + AuthToken);
    http.addHeader("Connection", "close");

    // Buat payload JSON
    String mac = macAddress.length() > 0 ? macAddress : WiFi.macAddress();
    String jsonPayload = "{\"mac_address\":\"" + mac + "\", \"status\":\"2\"}";

    int httpResponseCode = http.POST(jsonPayload);
    Serial.println("Mengirim ke: " + url);
    Serial.println("Payload: " + jsonPayload);

    if (httpResponseCode == HTTP_CODE_OK)
    {
      String response = http.getString();
      response.trim();
      if (!response.startsWith("<")) {
        Serial.println("HTTP Response code: " + String(httpResponseCode));
      }
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
        }
      }
      else if (errResp.length() > 0)
      {
        Serial.println("Respon Error: " + errResp);
      }
      Serial.println("Gagal mengirim data, coba lagi...");
    }
    http.end();
    delay(5000);
  }
}


bool sendHistoryRfid(String rfid) {
    if ((WiFi.status() == WL_CONNECTED || isEthernetConnected()) && !updateHistoryInProgress) {
        updateHistoryInProgress = true; // Set status pengiriman berlangsung

        Serial.println("Pengiriman data dimulai, modul RFID dinonaktifkan...");
        HTTPClient http;
        String url = String(ServerName) + String(ApiSendHistory);
        http.begin(url);
        http.setTimeout(5000);
        http.addHeader("Accept", "application/json");
        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-Requested-With", "XMLHttpRequest");
        http.addHeader("Authorization", "Bearer " + AuthToken);
        http.addHeader("Connection", "close");

        // Buat payload JSON
        String jsonPayload = "{\"uid\":\"" + rfid + "\", \"mac_address\":\"" + macAddress + "\"}";

        int httpResponseCode = http.POST(jsonPayload);
        Serial.println("Mengirim ke: " + url);

        bool result = false; // Default gagal

        if (httpResponseCode == HTTP_CODE_OK) {
            String response = http.getString();
            response.trim();
            if (!response.startsWith("<")) {
                Serial.println("HTTP Response code: " + String(httpResponseCode));
                Serial.println("Response: " + response);
            }
            result = true; // Pengiriman sukses
        } else {
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
                }
            }
            else if (errResp.length() > 0)
            {
                Serial.println("Respon Error: " + errResp);
            }
            Serial.println("Kirim akses gagal...");
        }

        http.end();
        updateHistoryInProgress = false; // Set status pengiriman selesai

        return result; // Mengembalikan status pengiriman
    } else if (updateHistoryInProgress) {
        Serial.println("Proses pengiriman sedang berlangsung, menunggu selesai...");
    } else {
        Serial.println("Jaringan tidak terhubung, tidak dapat mengirim data.");
    }

    return false; // Mengembalikan false jika gagal
}