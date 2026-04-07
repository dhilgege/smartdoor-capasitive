#include <config/config.h>

// komunikasi
// const int RXp2 = 16;
// const int TXp2 = 17;

//rfid 16mhz
const int SS_PIN = 112; //gak dipake
const int RST_PIN = 111; //gak dipake

//LED Bawaan
const int LED_Red = 38; // default red
const int LED_Green = 47;
const int LED_Yellow = 48;

//LED External
const int EXT_LED_RED = 7;  //SPI_CS2
const int EXT_LED_GREEN = 17; //SPI_CS1
const int EXT_LED_BLUE = 17; //SPI_MOSI


// koneksi modul
const int DoorPin = 39;
const int BUZZER_PIN = 40;

// pin lainnya
const int detectPin = 12;
const int resetpin = 200;// belum dipakai

//rfid125
const int GWIOT_7941E_RX_PIN = 41;


String AuthToken;

// etc
String macAddress;
String SSID;
String status_device = "1"; // Status perangkat default

// Array untuk menyimpan data RFID
String rfids[1000];
int rfidCount = 0;

const char *ssidwifimanager = "Device IoT Ruang Aula Lt4";
const char *passwordwifimanager = "11111111";
const char *customHostname = ssidwifimanager;

//pin efault&yang diganti di respon/epproom
String StoredDoorPin = "111111"; // dfl

String date = "";

bool updateHistoryInProgress = false; // Status proses pengiriman data


unsigned long doorTimerStart = 0;
int doorTimerDuration = 0;
bool doorTimerActive = false;

volatile bool isOtaMode = false;

