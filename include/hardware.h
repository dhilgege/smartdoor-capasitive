#ifndef name
#define name

void pinSetup();

// Door
void ActivateDoor(bool state);
void ActivateDoorTimer(int time);

// LED
void blinkLED(bool startBlinking);
void TurnOnLed(String state);
void TurnOFFLed(String state);

// Buzzer
void playKeypadTone();
void playResetTone(int time);
void playSuccessTone();
void playDeleteTone();
void playErrorTone();
void playChangePWDTone();
void playDownloadTone();

// Etc



#endif