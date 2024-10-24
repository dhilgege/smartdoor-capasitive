#include <SPI.h>     //rfid
#include <MFRC522.h> //rfid
#include <config/apidata.h>
#include "wifi_manager.h"
#include "hardware.h"


// RFID
#define SS_PIN 10
#define RST_PIN 7
MFRC522 mfrc522(SS_PIN, RST_PIN);

extern int rfidCount;
extern String rfids[];


void setupRfid(){
  SPI.begin();        // Init SPI bus
mfrc522.PCD_Init(); // Init MFRC522
Serial.println("Ready");

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


void cekRfid(){
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
    Serial1.println("200"); // Buka kunci
    Serial.println("berhasil mengirim Serial 200");
  }
  else
  {
    Serial1.println("201"); // Akses ditolak
    Serial.println("berhasil mengirim Serial 201");
  }

  blinkLED(false);

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}
