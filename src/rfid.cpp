#include <Arduino.h>
#include "config/config.h"
#include "hardware.h"
#include "hitApi.h"
#include "rfid.h"

// =============================================================================
// DRIVER SPESIFIK BERDASARKAN PILIHAN MODUL (ACTIVE_RFID_MODULE)
// =============================================================================

#if (ACTIVE_RFID_MODULE == RFID_MODULE_RDM6300)
    #include <rdm6300.h>
    static Rdm6300 rdm6300;

#elif (ACTIVE_RFID_MODULE == RFID_MODULE_GWIOT7941E)
    #include <Gwiot7941e.h>
    static Gwiot7941e gwiot7941e;

#elif (ACTIVE_RFID_MODULE == RFID_MODULE_MFRC522)
    #include <SPI.h>
    #include <MFRC522.h>
    static MFRC522 mfrc522(SS_PIN, RST_PIN);

#else
    #error "Modul RFID belum ditentukan atau tidak valid di ACTIVE_RFID_MODULE (config.h)!"
#endif

// =============================================================================
// VERIFIKASI UID TERHADAP DATA SERVER
// =============================================================================
bool verifyRfid(const String &uidStr)
{
    for (int i = 0; i < rfidCount; i++)
    {
        if (rfids[i].equalsIgnoreCase(uidStr) || rfids[i] == uidStr)
        {
            return true; // UID cocok dengan salah satu data RFID yang diunduh
        }
    }
    return false; // UID tidak ditemukan
}

// =============================================================================
// PROSES AKSI KETIKA KARTU TERDETEKSI (LOGIKA BERSAMA)
// =============================================================================
static void processRfidTag(const String &uidStr)
{
    Serial.println("==============================");
    Serial.print("UID Terbaca: ");
    Serial.println(uidStr);
    Serial.println("==============================");

    TurnOnLed("GREEN");

    if (verifyRfid(uidStr))
    {
        playSuccessTone();
        ActivateDoorTimer(DEFAULT_DOOR_TIMER_MS);
        Serial.println("Berhasil buka kunci dengan RFID");

        // Kirim riwayat akses RFID ke server
        sendHistoryRfid(uidStr);
    }
    else
    {
        playErrorTone();
        Serial.println("RFID akses ditolak");
    }

    TurnOFFLed("GREEN");
}

// =============================================================================
// INISIALISASI MODUL RFID (SETUP)
// =============================================================================
void rfidSetup()
{
#if (ACTIVE_RFID_MODULE == RFID_MODULE_RDM6300)
    pinMode(LED_Red, OUTPUT);
    digitalWrite(LED_Red, LOW);

    Serial.println("Initializing RDM6300 (125 kHz)...");
    rdm6300.begin(RFID125_RX_PIN);
    Serial.println("Place RFID tag near RDM6300...");

#elif (ACTIVE_RFID_MODULE == RFID_MODULE_GWIOT7941E)
    pinMode(LED_Red, OUTPUT);
    digitalWrite(LED_Red, LOW);

    Serial.println("Initializing GWIOT-7941E (125 kHz)...");
    gwiot7941e.begin(GWIOT_7941E_RX_PIN);
    Serial.println("Place RFID tag near GWIOT-7941E...");

#elif (ACTIVE_RFID_MODULE == RFID_MODULE_MFRC522)
    // SPI Pin: SCK = IO12, MISO = IO13, MOSI = IO11, SS = SS_PIN
    SPI.begin(12, 13, 11, SS_PIN);
    mfrc522.PCD_Init();
    Serial.println("Initializing MFRC522 (13.56 MHz)...");
    Serial.println("MFRC522 RFID Ready");
#endif
}

// =============================================================================
// LOOP PEMBACAAN KARTU RFID
// =============================================================================
void readRfid()
{
    if (updateHistoryInProgress)
    {
        Serial.println("Sedang dalam proses update, pembacaan RFID ditunda...");
#if (ACTIVE_RFID_MODULE == RFID_MODULE_GWIOT7941E)
        gwiot7941e.end();
#endif
        delay(50);
        return;
    }

#if (ACTIVE_RFID_MODULE == RFID_MODULE_RDM6300)
    uint32_t tagId = rdm6300.get_new_tag_id();
    if (tagId)
    {
        String uidStr = String(tagId, HEX);
        uidStr.toLowerCase();
        processRfidTag(uidStr);
        delay(1500); // Cooldown setelah pembacaan
    }
    delay(10);

#elif (ACTIVE_RFID_MODULE == RFID_MODULE_GWIOT7941E)
    if (gwiot7941e.update())
    {
        String uidStr = String(gwiot7941e.getLastTagId(), HEX);
        uidStr.toLowerCase();
        processRfidTag(uidStr);
        delay(1500); // Cooldown setelah pembacaan
    }
    delay(50);

#elif (ACTIVE_RFID_MODULE == RFID_MODULE_MFRC522)
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        delay(10);
        return;
    }

    String uidStr = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        uidStr += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uidStr += String(mfrc522.uid.uidByte[i], HEX);
    }
    uidStr.toLowerCase();

    processRfidTag(uidStr);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(1500); // Cooldown setelah pembacaan
#endif
}

