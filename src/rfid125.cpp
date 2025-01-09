/**
 * A simple example to interface with Gwiot 7941E RFID reader.
 *
 * Connect the 7941E to VCC=5V, GND=GND, D0/TXD=any GPIO (this case GPIO-04)
 *
 * Note: for ESP* or other 3.3V based microcontroller,
 *       you must divide the D0/TXD out of the 7941E to the 3.3V levels,
 *       Here's a suggested resistor divider:
 *       GND --- <2k resistor> --- GPIO_2 --- <1k resistor> --- D0/TXD (7941E).
 *
 * Arad Eizen (https://github.com/arduino12)
 * Gutierrez PS (https://github.com/gutierrezps)
 */

#include <Gwiot7941e.h>
#include "hardware.h"
#include <hitApi.h>

#define GWIOT_7941E_RX_PIN 41
#define READ_LED_PIN 38

extern int rfidCount;
extern String rfids[];
extern bool updateHistoryInProgress;

String uidStr = ""; // tempat id rfid

Gwiot7941e gwiot7941e;

void rfid125Setup()
{

    pinMode(READ_LED_PIN, OUTPUT);
    digitalWrite(READ_LED_PIN, LOW);

    gwiot7941e.begin(GWIOT_7941E_RX_PIN);

    Serial.println("\nPlace RFID tag near the 7941E...");
}

bool verifyRfid125(const String &uidStr)
{
    for (int i = 0; i < rfidCount; i++)
    {
        if (rfids[i] == uidStr)
        {
            return true; // UID cocok dengan salah satu RFID yang di-download
        }
    }
    return false; // UID tidak ditemukan
}
void readRfid125()
{
    // Cek apakah tidak ada proses update yang sedang berlangsung
    if (!updateHistoryInProgress)
    {
        // Jika ada tag_id non-zero, update() akan mengembalikan true - sebuah tag terdeteksi!
        if (gwiot7941e.update())
        {
            uidStr = String(gwiot7941e.getLastTagId(), HEX);
            Serial.print("UID Terbaca: ");
            Serial.println(uidStr);
            TurnOnLed("GREEN");

            // Verifikasi UID dengan data RFID yang di-download
            if (verifyRfid125(uidStr))
            {
                playSuccessTone();
                ActivateDoorTimer(3000);
                TurnOFFLed("GREEN");
                Serial.println("Berhasil buka kunci dengan RFID");

                // Kirim data histori setelah membuka kunci
                sendHistoryRfid(uidStr);
            }
            else
            {
                playErrorTone();
                TurnOFFLed("GREEN");
                Serial.println("RFID akses ditolak");
            }
        }
    }
    else
    {
        Serial.println("Sedang dalam proses update, pembacaan RFID ditunda...");
        gwiot7941e.end();
    }

    delay(50);
}
