#ifndef GLOBAL_VAR
#define GLOBAL_VAR
#include <Arduino.h>

// extern const int RXp2;
// extern const int TXp2;

// led bawaan
extern const int LED_Red;
extern const int LED_Green;
extern const int LED_Yellow;


// led external
extern const int EXT_LED_RED;
extern const int EXT_LED_GREEN;
extern const int EXT_LED_BLUE;

//unused rfid 16mhz
extern const int SS_PIN;
extern const int RST_PIN;

// etc
extern const int DoorPin;
extern const int BUZZER_PIN;
extern const int detectPin;
extern const int resetpin;

extern const int GWIOT_7941E_RX_PIN;


extern String AuthToken;
extern String macAddress;
extern String SSID;
extern String status_device;
extern int rfidCount;
extern String rfids[];

extern const char *ssidwifimanager;
extern const char *passwordwifimanager;
extern const char *customHostname;


extern String StoredDoorPin;

extern String date;

extern unsigned long doorTimerStart;
extern int doorTimerDuration;
extern bool doorTimerActive;

extern volatile bool isOtaMode;


#endif
