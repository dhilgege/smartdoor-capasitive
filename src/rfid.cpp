//UNUSED FILE, WORKING BUT NOT IN USE


#include <SPI.h>     //rfid
#include <MFRC522.h> //rfid
#include "hardware.h"
#include "config/config.h"

extern const int SS_PIN;
extern const int RST_PIN;

// RFID

MFRC522 mfrc522(SS_PIN, RST_PIN);

extern int rfidCount;
extern String rfids[];

void setupRfid()
{
  SPI.begin(12, 13, 11, SS_PIN); // SCK = IO12, MISO = IO13, MOSI = IO11, SS = IO10
  mfrc522.PCD_Init();            // Inisialisasi MFRC522
  Serial.println("RFID Ready");
}

bool verifyRfid(const String &uidStr)
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

void cekRfid()
{
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  blinkLED(true);

  // Prepare UID string
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    uidStr += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }

  // Convert to upper case
  uidStr.toUpperCase();

  Serial.print("Card UID: ");
  Serial.println(uidStr);

  // Verifikasi UID dengan data RFID yang di-download
  if (verifyRfid(uidStr))
  {
    ActivateDoorTimer(0);
    playSuccessTone();
    Serial.println("berhasil buka kunci dengan rfid");
  }
  else
  {
    playErrorTone();
    Serial.println("rfid akses ditolak");
  }

  blinkLED(false);

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}
