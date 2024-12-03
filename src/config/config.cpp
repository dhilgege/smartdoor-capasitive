#include <config/config.h>

// komunikasi
// const int RXp2 = 16;
// const int TXp2 = 17;

// koneksi modul
const int LED_Red = 38; // default red 
const int LED_Green = 47; 
const int LED_Yellow = 48; 
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

const char *ssidwifimanager = "Smartdoor003";
const char *passwordwifimanager = "11111111";
const char *customHostname = "smartdoor003";

//pin efault&yang diganti di respon/epproom
String StoredDoorPin = "111111"; // dfl

String date = "";

bool updateHistoryInProgress = false; // Status proses pengiriman data
