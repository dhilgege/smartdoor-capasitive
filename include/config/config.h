#ifndef GLOBAL_VAR
#define GLOBAL_VAR
#include <Arduino.h> 

// extern const int RXp2;
// extern const int TXp2;
extern const int LED_PIN;
extern const int DoorPin;
extern const int BUZZER_PIN;
extern const int detectPin;
extern const int resetpin;

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

#endif
