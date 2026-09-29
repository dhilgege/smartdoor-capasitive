#include <Arduino.h>
#include "config/config.h"
#include "hardware.h"
#include "wifi_manager.h"
#include "otaHandleMain.h"
#include "hitApi.h"

// =============================================================================
// DRIVER SPESIFIK BERDASARKAN TIPE KEYPAD (ACTIVE_KEYPAD_TYPE)
// =============================================================================

#if (ACTIVE_KEYPAD_TYPE == KEYPAD_TYPE_MATRIX_4X4)
// -----------------------------------------------------------------------
// Mode Keypad Mekanik 4x4 (Library Keypad)
// -----------------------------------------------------------------------
/* @file MultiKey.ino
|| @version 1.0
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
|| @description
|| | The latest version, 3.0, of the keypad library supports up to 10
|| | active keys all being pressed at the same time. This sketch is an
|| | example of how you can get multiple key presses from a keypad or
|| | keyboard.
|| #
*/
#include <Keypad.h>

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

Keypad kpd = Keypad(makeKeymap(keys), KEYPAD_ROW_PINS, KEYPAD_COL_PINS, KEYPAD_ROWS, KEYPAD_COLS);

void keypadSetup()
{
    // Library Keypad tidak membutuhkan inisialisasi khusus
    Serial.println("[Keypad] Mode: Mekanik 4x4 Matrix");
}

// Baca satu key dari keypad mekanik
static char readKeypadKey()
{
    return kpd.getKey();
}

#elif (ACTIVE_KEYPAD_TYPE == KEYPAD_TYPE_I2C_TOUCH_4X3)
// -----------------------------------------------------------------------
// Mode Keypad Capacitive Touch I2C 4x3 (MPR121)
// Implementasi berdasarkan kode test yang berhasil di "kepad testingg"
// Pin: SDA=GPIO8, SCL=GPIO9, ADDR=0x5A
// -----------------------------------------------------------------------
#include <Wire.h>
#include <Adafruit_MPR121.h>

static Adafruit_MPR121 mpr121      = Adafruit_MPR121();
static uint16_t        prevTouched = 0;
static bool            mpr121Ready = false;

// Threshold sensitivitas sentuhan (touch=6, release=3)
#define MPR121_TOUCH_TH    6
#define MPR121_RELEASE_TH  3

// =======================================================================
// Pemetaan layout keypad 4 kolom x 3 baris:
// Baris 1: Elektroda 0,  1,  2,  3 -> '1', '2', '3', 'A'
// Baris 2: Elektroda 4,  5,  6,  7 -> '4', '5', '6', 'B'
// Baris 3: Elektroda 8,  9, 10, 11 -> '7', '8', '9', 'C'
// =======================================================================
static const char KEYPAD_LAYOUT_4X3[12] = {
    '7', '8', '9', 'C',   // ELE 8, 9, 10, 11
    '4', '5', '6', 'B',   // ELE 4, 5, 6, 7
    '1', '2', '3', 'A',   // ELE 0, 1, 2, 3
};

// -----------------------------------------------------------------------
// Diagnostik: I2C Scanner (dari test project)
// -----------------------------------------------------------------------
static void i2cScan()
{
    Serial.println("\n[I2C Scan] Mencari perangkat I2C...");
    int found = 0;
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[I2C Scan] Ditemukan: 0x%02X", addr);
            if      (addr == 0x5A) Serial.print(" <- MPR121 (ADDR=GND)");
            else if (addr == 0x5B) Serial.print(" <- MPR121 (ADDR=3V3)");
            else if (addr == 0x5C) Serial.print(" <- MPR121 (ADDR=SDA)");
            else if (addr == 0x5D) Serial.print(" <- MPR121 (ADDR=SCL)");
            Serial.println();
            found++;
        }
    }
    if (found == 0)
        Serial.println("[I2C Scan] Tidak ada perangkat ditemukan! Cek kabel SDA/SCL/VCC/GND.");
    else
        Serial.printf("[I2C Scan] Total: %d perangkat ditemukan.\n\n", found);
}

// -----------------------------------------------------------------------
// Diagnostik: tampilkan data Filtered dan Baseline tiap elektroda
// -----------------------------------------------------------------------
static void printMpr121Diagnostics()
{
    if (!mpr121Ready) return;

    Serial.println("\n--- [DIAGNOSTIK ELEKTRODA 0-11] ---");
    Serial.print("Filtered: ");
    for (uint8_t i = 0; i < 12; i++)
        Serial.printf("[%d]:%-4d ", i, mpr121.filteredData(i));
    Serial.println();

    Serial.print("Baseline: ");
    for (uint8_t i = 0; i < 12; i++)
        Serial.printf("[%d]:%-4d ", i, mpr121.baselineData(i));
    Serial.println("\n------------------------------------\n");
}

void keypadSetup()
{
    Serial.println("[Keypad] Mode: Capacitive Touch I2C 4x3 (MPR121)");

    // Inisialisasi I2C pada pin kustom (SDA=GPIO8, SCL=GPIO9) dengan clock 100kHz
    Wire.begin(KEYPAD_I2C_SDA_PIN, KEYPAD_I2C_SCL_PIN);
    Wire.setClock(100000);
    Wire.setTimeOut(100);

    // Scan I2C untuk diagnostik koneksi
    i2cScan();

    Serial.println("============================================");
    Serial.println("  MPR121 Capacitive Keypad -- SmartDoor    ");
    Serial.println("============================================");
    Serial.printf("[Config] SDA=GPIO%d | SCL=GPIO%d | ADDR=0x%02X\n",
                  KEYPAD_I2C_SDA_PIN, KEYPAD_I2C_SCL_PIN, KEYPAD_I2C_ADDR);
    Serial.printf("[Config] Touch Threshold=%d | Release Threshold=%d\n",
                  MPR121_TOUCH_TH, MPR121_RELEASE_TH);

    // Inisialisasi MPR121 dengan threshold sensitif
    if (!mpr121.begin(KEYPAD_I2C_ADDR, &Wire, MPR121_TOUCH_TH, MPR121_RELEASE_TH))
    {
        Serial.println("[Keypad] ERROR: MPR121 tidak ditemukan! Periksa koneksi SDA/SCL/VCC/GND.");
        Serial.printf("[Keypad] SDA=GPIO%d, SCL=GPIO%d, ADDR=0x%02X\n",
                      KEYPAD_I2C_SDA_PIN, KEYPAD_I2C_SCL_PIN, KEYPAD_I2C_ADDR);
        mpr121Ready = false;
        return;
    }

    // Aktifkan auto-config agar MPR121 mengkalibrasi arus & waktu charge tiap elektroda
    mpr121.setAutoconfig(true);
    mpr121Ready = true;

    Serial.println("[OK] MPR121 berhasil terhubung & autoconfig aktif!");
    Serial.printf("[Keypad] MPR121 OK & Autoconfig Aktif. SDA=GPIO%d, SCL=GPIO%d, ADDR=0x%02X\n",
                  KEYPAD_I2C_SDA_PIN, KEYPAD_I2C_SCL_PIN, KEYPAD_I2C_ADDR);

    delay(500);
    printMpr121Diagnostics();
}

// -----------------------------------------------------------------------
// Baca satu key dari keypad I2C capacitive touch.
// Menggunakan loop per-elektroda (dari test project) untuk deteksi
// press & release yang lebih akurat dibanding bitmask sekaligus.
// -----------------------------------------------------------------------
static char readKeypadKey()
{
    if (!mpr121Ready)
        return 0;

    uint16_t curTouched = mpr121.touched();

    // Deteksi error I2C (0x0FFF = bus error) - throttled 2 detik
    if (curTouched == 0x0FFF) {
        static unsigned long lastI2cErr = 0;
        if (millis() - lastI2cErr > 2000) {
            Serial.println("[WARNING] I2C Bus Error (0x0FFF) - periksa koneksi kabel SDA/SCL!");
            lastI2cErr = millis();
        }
        return 0;
    }

    char pressedKey = 0;

    // Periksa transisi per-elektroda (0 sampai 11)
    for (uint8_t i = 0; i < 12; i++)
    {
        bool nowTouched = (curTouched  & (1 << i)) != 0;
        bool wasTouched = (prevTouched & (1 << i)) != 0;

        // Event: Baru disentuh (rising edge) - ambil tombol pertama saja
        if (nowTouched && !wasTouched && pressedKey == 0)
        {
            pressedKey = KEYPAD_LAYOUT_4X3[i];
            Serial.printf("[Keypad] Elektroda %d ditekan -> '%c'\n", i, pressedKey);
        }

        // Event: Dilepas (falling edge)
        if (!nowTouched && wasTouched)
        {
            Serial.printf("[Keypad] Elektroda %d dilepas.\n", i);
        }
    }

    prevTouched = curTouched;
    return pressedKey;
}

#else
    #error "ACTIVE_KEYPAD_TYPE tidak valid! Pilih KEYPAD_TYPE_MATRIX_4X4 atau KEYPAD_TYPE_I2C_TOUCH_4X3 di config.h."
#endif

// =============================================================================
// INPUT PIN STATE (RUNTIME)
// =============================================================================

static String inputPin = "";

// =============================================================================
// LOGIKA PEMROSESAN TOMBOL (BERSAMA untuk kedua mode)
// =============================================================================

void handleKeypad()
{
    char key = readKeypadKey();

    if (!key)
        return;

    // Tombol Reset/Delete: 'D' (4x4 matrix) atau '*' / 'C' (4x3 touch)
    if (key == 'D' || key == '*' || key == 'C')
    {
        inputPin = "";
        Serial.println("[Keypad] Input direset.");
        playDeleteTone();
    }
    else
    {
        inputPin += key;
        Serial.print("[Keypad] Input: ");
        Serial.println(inputPin);
        playKeypadToneForKey(key); // Nada unik per tombol (tangga nada musik)

        // Jika sudah 6 digit, periksa PIN
        if (inputPin.length() == 6)
        {
            if (inputPin == StoredDoorPin)
            {
                Serial.println("[Keypad] PIN benar! Akses diizinkan.");
                ActivateDoorTimer(3000);
                playSuccessTone();
            }
            else if (inputPin == "##01##")
            {
                playResetTone(200);
                Serial.println("[Keypad] Restart Device");
                delay(500);
                ESP.restart();
            }
            else if (inputPin == "##02##")
            {
                playResetTone(200);
                Serial.println("[Keypad] Reset EEPROM ESP");
                delay(500);
                resetEpproomWifi();
            }
            else if (inputPin == "##03##")
            {
                playResetTone(200);
                Serial.println("[Keypad] Resetting Wifi & Setup Wifi");
                delay(500);
                resetEpproomWifi();
                setupWifi();
            }
            else if (inputPin == "##04##")
            {
                playResetTone(200);
                Serial.println("[Keypad] Force OTA calling");
                delay(500);
                enterOtaMode();
                setupWifi();
            }
            else if (inputPin == "##05##")
            {
                playResetTone(200);
                Serial.println("[Keypad] Request get RFID");
                delay(500);
                getAllRfid();
            }
            else
            {
                playErrorTone();
                Serial.println("[Keypad] PIN salah! Coba lagi.");
            }
            inputPin = ""; // Reset input setelah evaluasi
        }
    }
}

// =============================================================================
// TASK FREERTOS
// =============================================================================

void KeypadTask(void *pvParameters)
{
    // Inisialisasi keypad sesuai tipe yang aktif
    keypadSetup();

    for (;;)
    {
        if (isOtaMode)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        if (doorTimerActive && millis() - doorTimerStart >= doorTimerDuration)
        {
            ActivateDoor(false);
            Serial.println("[Keypad] Door Closed");
            doorTimerActive = false;
        }

        handleKeypad();

        vTaskDelay(50 / portTICK_PERIOD_MS); // 50ms polling (lebih responsif)
    }
}
