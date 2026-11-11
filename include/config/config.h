#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <config/apidata.h>
#include <config/version.h>

// =============================================================================
// 0a. PILIHAN MODUL RFID (RFID MODULE SELECTOR)
// Pilih salah satu modul RFID yang digunakan pada board:
//   RFID_MODULE_RDM6300    : Modul 125 kHz RDM6300 (Default)
//   RFID_MODULE_GWIOT7941E : Modul 125 kHz GWIOT-7941E
//   RFID_MODULE_MFRC522    : Modul 13.56 MHz MFRC522 (RC522 SPI)
// =============================================================================
#define RFID_MODULE_RDM6300     1
#define RFID_MODULE_GWIOT7941E  2
#define RFID_MODULE_MFRC522     3

// Ubah baris ini untuk memilih modul yang aktif pada hardware:
#define ACTIVE_RFID_MODULE      RFID_MODULE_RDM6300

// =============================================================================
// 0b. PILIHAN TIPE KEYPAD (KEYPAD TYPE SELECTOR)
// Pilih salah satu tipe keypad yang digunakan pada board:
//   KEYPAD_TYPE_MATRIX_4X4    : Keypad mekanik 4x4 matrix (Default)
//   KEYPAD_TYPE_I2C_TOUCH_4X3 : Keypad capacitive touch I2C 4x3 (MPR121)
// =============================================================================
#define KEYPAD_TYPE_MATRIX_4X4     1
#define KEYPAD_TYPE_I2C_TOUCH_4X3  2

// Ubah baris ini untuk memilih tipe keypad yang aktif pada hardware:
#define ACTIVE_KEYPAD_TYPE         KEYPAD_TYPE_I2C_TOUCH_4X3

// =============================================================================
// 1. PENGATURAN WIFI & ACCESS POINT (WiFiManager)
// =============================================================================
extern const char *ssidwifimanager;       // SSID Hotspot konfigurasi WiFi
extern const char *passwordwifimanager;   // Password Hotspot konfigurasi WiFi
extern const char *customHostname;        // Hostname perangkat ESP32

// =============================================================================
// 2. PEMETAAN PIN GPIO (HARDWARE PIN MAPPING)
// Catatan: Nilai pin tetap sama persis untuk menjaga kompatibilitas hardware
// =============================================================================

// --- LED Internal (Bawaan ESP32) ---
extern const int LED_Red;                 // GPIO 38: LED Merah
extern const int LED_Green;               // GPIO 47: LED Hijau
extern const int LED_Yellow;              // GPIO 48: LED Kuning

// --- LED Eksternal (RGB / Status) ---
extern const int EXT_LED_RED;             // GPIO 7
extern const int EXT_LED_GREEN;           // GPIO 17
extern const int EXT_LED_BLUE;            // GPIO 17

// --- Relay / Solenoid Door & Buzzer ---
extern const int DoorPin;                 // GPIO 39: Relay Kunci Pintu
extern const int BUZZER_PIN;              // GPIO 40: Buzzer Notifikasi Audio

// --- Tombol Exit (Push On / NO) ---
extern const int EXIT_BTN_PIN;            // GPIO 13: J3 pin 6 (SPI_MISO), kaki lain ke GND (J3 pin 7)

// --- Sensor & Tombol Deteksi ---
extern const int detectPin;               // GPIO 12: Sensor Deteksi Pintu (Input Pullup)
extern const int resetpin;                // GPIO 200: Pin Reset Tambahan (Belum Digunakan)

// --- W5500 Ethernet SPI ---
extern const int W5500_MOSI;
extern const int W5500_MISO;
extern const int W5500_SCLK;
extern const int W5500_CS;
extern const int W5500_RST;
extern const int W5500_INT;

// --- RFID 125 kHz (RDM6300 / GWIOT 7941E) ---
extern const int GWIOT_7941E_RX_PIN;      // GPIO 41: UART RX RFID 125kHz
extern const int RFID125_RX_PIN;          // GPIO 41: Alias untuk RDM6300 RX

// --- RFID 13.56 MHz (MFRC522 RC522 - Cadangan) ---
extern const int SS_PIN;                  // GPIO 112: SPI SS/CS (Tidak digunakan saat ini)
extern const int RST_PIN;                 // GPIO 111: RST (Tidak digunakan saat ini)

// --- Keypad 4x4 Matrix ---
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
extern byte KEYPAD_ROW_PINS[KEYPAD_ROWS]; // Baris: {6, 5, 2, 1}
extern byte KEYPAD_COL_PINS[KEYPAD_COLS]; // Kolom: {17, 16, 15, 14}

// --- Keypad Capacitive Touch I2C 4x3 (MPR121) ---
extern const int KEYPAD_I2C_SDA_PIN;  // GPIO 8: SDA
extern const int KEYPAD_I2C_SCL_PIN;  // GPIO 9: SCL
extern const int KEYPAD_I2C_IRQ_PIN;  // GPIO 17: IRQ (Interrupt)
#define KEYPAD_I2C_ADDR  0x5A         // Alamat I2C default MPR121
extern char KEYPAD_I2C_TOUCH_KEYS[12]; // Pemetaan 12 elektroda ke karakter

// =============================================================================
// 3. PARAMETER WAKTU & TIMING DEFAULT
// =============================================================================
const int DEFAULT_DOOR_TIMER_MS = 3000;   // Durasi buka pintu otomatis (ms)

// --- Konfigurasi Real-Time Clock / NTP ---
extern const char *ntpServer1;
extern const char *ntpServer2;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;
extern const char *time_zone;

// =============================================================================
// 4. VARIABEL RUNTIME GLOBAL (SYSTEM STATE)
// =============================================================================
extern String AuthToken;                  // Token otentikasi API
extern String macAddress;                 // MAC Address perangkat
extern String SSID;                       // SSID WiFi yang terhubung
extern String status_device;              // Status perangkat dari API (default: "1")
extern String StoredDoorPin;              // PIN akses pintu (default: "111111")
extern String date;                       // String tanggal / jam sinkronisasi

// Status sinkronisasi RFID
extern String rfids[];                    // Cache UID RFID yang diizinkan
extern int rfidCount;                     // Jumlah kartu RFID tersimpan
extern bool updateHistoryInProgress;      // Flag status pengiriman riwayat ke server

// State Timer Pintu
extern unsigned long doorTimerStart;      // Timestamp saat pintu terbuka
extern int doorTimerDuration;             // Durasi timer pintu aktif
extern bool doorTimerActive;              // Status apakah timer pintu sedang aktif

// State Mode OTA
extern volatile bool isOtaMode;           // Flag apakah perangkat sedang dalam mode OTA

#endif // CONFIG_H