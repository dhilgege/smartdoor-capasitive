#include <config/config.h>

// =============================================================================
// 1. PENGATURAN WIFI & ACCESS POINT (WiFiManager)
// Ubah pengaturan ini jika ingin mengganti SSID/Password AP konfigurasi
// =============================================================================
const char *ssidwifimanager     = "Smartdoor mesh LT4";
const char *passwordwifimanager = "11111111";
const char *customHostname     = ssidwifimanager;

// =============================================================================
// 2. PEMETAAN PIN GPIO (HARDWARE PIN MAPPING)
// PENTING: Jangan ubah nomor pin jika PCB/rangkaian hardware tidak berubah!
// =============================================================================

// --- LED Internal (Bawaan Modul ESP32) ---
const int LED_Red    = 4; // Aktif HIGH
const int LED_Green  = 47; // Aktif HIGH
const int LED_Yellow = 48; // Aktif HIGH

// --- LED Eksternal (RGB Indikator) ---
const int EXT_LED_RED   = 7;  // SPI_CS2
const int EXT_LED_GREEN = 17; // SPI_CS1
const int EXT_LED_BLUE  = 17; // SPI_MOSI

// --- Relay / Solenoid Pintu & Buzzer ---
const int DoorPin    = 18; // Pin kontrol relay pintu (aktif HIGH)
const int BUZZER_PIN = 21; // Pin buzzer nada audio

// --- Tombol Exit (Push On / NO) ---
// J3 pin 6 (SPI_MISO), kaki lain tombol ke GND (J3 pin 7). Pakai INPUT_PULLUP.
const int EXIT_BTN_PIN = 13;

// --- Sensor & Tombol Deteksi ---
const int detectPin = 12;  // Sensor deteksi pintu
const int resetpin  = 200; // Reserved (belum dipakai)

// --- W5500 Ethernet SPI ---
const int W5500_MOSI = 35;
const int W5500_MISO = 37;
const int W5500_SCLK = 36;
const int W5500_CS   = 39; 
const int W5500_RST  = 38; 
const int W5500_INT  = 40; // ESP-IDF driver membutuhkan pin INT untuk menerima DHCP/paket

// --- RFID 125 kHz (RDM6300 / GWIOT 7941E) ---
const int GWIOT_7941E_RX_PIN = 41;
const int RFID125_RX_PIN     = 41; // Alias untuk RDM6300

// --- RFID 13.56 MHz (MFRC522 RC522 - Cadangan) ---
const int SS_PIN  = 112; // Unused
const int RST_PIN = 111; // Unused

// --- Keypad 4x4 Matrix ---
// Urutan koneksi pin keypad matrix 4x4
byte KEYPAD_ROW_PINS[KEYPAD_ROWS] = {6, 5, 2, 1};     // Row pins
byte KEYPAD_COL_PINS[KEYPAD_COLS] = {17, 16, 15, 14}; // Column pins

// --- Keypad Capacitive Touch I2C 4x3 (MPR121) ---
// SDA = GPIO 8, SCL = GPIO 9, IRQ = GPIO 17
const int KEYPAD_I2C_SDA_PIN = 8;
const int KEYPAD_I2C_SCL_PIN = 9;
const int KEYPAD_I2C_IRQ_PIN = 17;
// Pemetaan 12 elektroda MPR121 (ELE0-ELE11) ke karakter keypad 4x3:
// Baris 1: 1 2 3 | Baris 2: 4 5 6 | Baris 3: 7 8 9 | Baris 4: * 0 #
char KEYPAD_I2C_TOUCH_KEYS[12] = {
    '1', '2', '3',
    '4', '5', '6',
    '7', '8', '9',
    '*', '0', '#'
};

// =============================================================================
// 3. PARAMETER WAKTU & TIMING DEFAULT
// =============================================================================
const char *ntpServer1          = "pool.ntp.org";
const char *ntpServer2          = "time.nist.gov";
const long gmtOffset_sec        = 25200; // GMT+7 (WIB: 7 * 3600 detik)
const int daylightOffset_sec    = 0;
const char *time_zone           = "CET-1CEST,M3.5.0,M10.5.0/3";

// =============================================================================
// 4. VARIABEL RUNTIME GLOBAL (SYSTEM STATE)
// Nilai-nilai ini diatur dan diperbarui saat runtime berjalan
// =============================================================================
String AuthToken = "";
String macAddress = "";
String SSID = "";
String status_device = "1"; // Status perangkat default

// Array cache UID RFID dari server
String rfids[1000];
int rfidCount = 0;

// PIN pintu tersimpan (default awal sebelum update dari server/EEPROM)
String StoredDoorPin = "111111";

String date = "";
bool updateHistoryInProgress = false; // Flag proses pengiriman data RFID ke server

// State Timer Pintu
unsigned long doorTimerStart = 0;
int doorTimerDuration = 0;
bool doorTimerActive = false;

// State Mode Firmware OTA
volatile bool isOtaMode = false;