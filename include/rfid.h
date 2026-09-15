#ifndef RFID_H
#define RFID_H

#include <Arduino.h>

// Fungsi utama RFID (mendukung RDM6300, GWIOT-7941E, dan MFRC522)
void rfidSetup();
void readRfid();
bool verifyRfid(const String &uidStr);

// Alias kompatibilitas ke belakang
inline void setupRfid()    { rfidSetup(); }
inline void cekRfid()      { readRfid(); }
inline void rfid125Setup() { rfidSetup(); }
inline void readRfid125()  { readRfid(); }
inline bool verifyRfid125(const String &uidStr) { return verifyRfid(uidStr); }

#endif // RFID_H