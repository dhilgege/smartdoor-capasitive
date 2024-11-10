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


#define GWIOT_7941E_RX_PIN 41
#define READ_LED_PIN 38

extern int rfidCount;
extern String rfids[];

String uidStr = ""; // tempat id rfid


Gwiot7941e gwiot7941e;

void rfid125Setup()
{
    Serial.begin(115200);

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
    // if non-zero tag_id, update() returns true - a tag was read!
    if (gwiot7941e.update())
    {
        uidStr = String(gwiot7941e.getLastTagId(), HEX); 
        Serial.print("UID Terbaca: ");
        Serial.println(uidStr);

        // Verifikasi UID dengan data RFID yang di-download
        if (verifyRfid125(uidStr))
        {
            playSuccessTone();
            ActivateDoorTimer(3000);
            Serial.println("berhasil buka kunci dengan rfid");
        }
        else
        {
            playErrorTone();
            Serial.println("rfid akses ditolak");
        }
    }

    delay(50); 
}

