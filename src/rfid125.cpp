/**
 * RDM6300 RFID reader interface
 */

#include <rdm6300.h>
#include "hardware.h"
#include <hitApi.h>

#define RDM6300_RX_PIN 41
#define READ_LED_PIN 38

extern int rfidCount;
extern String rfids[];
extern bool updateHistoryInProgress;

String uidStr = "";

Rdm6300 rdm6300;

void rfid125Setup()
{
    pinMode(READ_LED_PIN, OUTPUT);
    digitalWrite(READ_LED_PIN, LOW);

    Serial.println("Initializing RDM6300...");

    rdm6300.begin(RDM6300_RX_PIN);

    Serial.println("Place RFID tag near the RDM6300...");
}

bool verifyRfid125(const String &uidStr)
{
    for (int i = 0; i < rfidCount; i++)
    {
        if (rfids[i] == uidStr)
        {
            return true;
        }
    }
    return false;
}

void readRfid125()
{
    if (!updateHistoryInProgress)
    {
        uint32_t tagId = rdm6300.get_new_tag_id();

        if (tagId)
        {
            uidStr = String(tagId, HEX);
            uidStr.toLowerCase();

            Serial.println("==============================");
            Serial.print("UID Terbaca: ");
            Serial.println(uidStr);
            Serial.println("==============================");

            TurnOnLed("GREEN");

            if (verifyRfid125(uidStr))
            {
                playSuccessTone();
                ActivateDoorTimer(3000);

                Serial.println("Berhasil buka kunci dengan RFID");

                sendHistoryRfid(uidStr);
            }
            else
            {
                playErrorTone();
                Serial.println("RFID akses ditolak");
            }

            TurnOFFLed("GREEN");

            delay(1500);
        }
    }
    else
    {
        Serial.println("Sedang dalam proses update, pembacaan RFID ditunda...");
    }

    delay(10);
}