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
    String cleanUid = uidStr;
    cleanUid.toLowerCase();
    cleanUid.trim();

    for (int i = 0; i < rfidCount; i++)
    {
        String serverUid = rfids[i];
        serverUid.toLowerCase();
        serverUid.trim();

        if (serverUid == cleanUid || serverUid.equalsIgnoreCase(cleanUid))
        {
            return true; // UID cocok dengan salah satu data RFID yang diunduh
        }

        // Fleksibilitas format jika salah satu memiliki leading zero (misal "00a1b2c3" vs "a1b2c3")
        String s1 = serverUid;
        String s2 = cleanUid;
        while (s1.startsWith("0") && s1.length() > 1) s1.remove(0, 1);
        while (s2.startsWith("0") && s2.length() > 1) s2.remove(0, 1);
        if (s1.length() > 0 && s1 == s2)
        {
            return true;
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
static uint32_t lastProcessedTag = 0;
static unsigned long lastTagProcessTime = 0;
const unsigned long RFID_COOLDOWN_MS = 2000; // Cooldown 2 detik untuk kartu yang sama

void readRfid()
{
    if (updateHistoryInProgress)
    {
        return;
    }

#if (ACTIVE_RFID_MODULE == RFID_MODULE_RDM6300)
    uint32_t tagId = rdm6300.get_tag_id();

    // Jika tidak ada kartu di dekat reader
    if (tagId == 0)
    {
        // Reset kartu terakhir setelah kartu diangkat selama minimal 500ms
        if (lastProcessedTag != 0 && (millis() - lastTagProcessTime > 500))
        {
            lastProcessedTag = 0;
        }
        return;
    }

    // Jika ada kartu terdeteksi:
    // Baca jika kartu berbeda ATAU kartu yang sama tetapi sudah lewat waktu cooldown
    if (tagId != lastProcessedTag || (millis() - lastTagProcessTime >= RFID_COOLDOWN_MS))
    {
        lastProcessedTag = tagId;
        lastTagProcessTime = millis();

        String uidStr = String(tagId, HEX);
        uidStr.toLowerCase();
        processRfidTag(uidStr);
    }

#elif (ACTIVE_RFID_MODULE == RFID_MODULE_GWIOT7941E)
    if (gwiot7941e.update())
    {
        uint32_t tagId = gwiot7941e.getLastTagId();
        if (tagId != 0 && (tagId != lastProcessedTag || (millis() - lastTagProcessTime >= RFID_COOLDOWN_MS)))
        {
            lastProcessedTag = tagId;
            lastTagProcessTime = millis();

            String uidStr = String(tagId, HEX);
            uidStr.toLowerCase();
            processRfidTag(uidStr);
        }
    }
    else
    {
        if (lastProcessedTag != 0 && (millis() - lastTagProcessTime > 500))
        {
            lastProcessedTag = 0;
        }
    }

#elif (ACTIVE_RFID_MODULE == RFID_MODULE_MFRC522)
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        if (lastProcessedTag != 0 && (millis() - lastTagProcessTime > 500))
        {
            lastProcessedTag = 0;
        }
        return;
    }

    uint32_t tagId = 0;
    for (byte i = 0; i < mfrc522.uid.size && i < 4; i++)
    {
        tagId = (tagId << 8) | mfrc522.uid.uidByte[i];
    }

    if (tagId != lastProcessedTag || (millis() - lastTagProcessTime >= RFID_COOLDOWN_MS))
    {
        lastProcessedTag = tagId;
        lastTagProcessTime = millis();

        String uidStr = "";
        for (byte i = 0; i < mfrc522.uid.size; i++)
        {
            uidStr += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
            uidStr += String(mfrc522.uid.uidByte[i], HEX);
        }
        uidStr.toLowerCase();

        processRfidTag(uidStr);
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
#endif
}

