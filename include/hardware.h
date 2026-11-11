#ifndef HARDWARE_H
#define HARDWARE_H

void pinSetup();

// Door
void ActivateDoor(bool state);
void ActivateDoorTimer(int time);

// Tombol exit (push on) -> buka pintu
void initExitButton();

// LED
extern bool isSystemError;
void initErrorIndicator();
void initNetworkIndicator();
void setErrorState(bool state);
void blinkLED(bool startBlinking);
void TurnOnLed(String state);
void TurnOFFLed(String state);

// Buzzer
void playKeypadTone();
void playKeypadToneForKey(char key); // Nada unik per tombol (tangga nada musik)
void playResetTone(int time);
void playSuccessTone();
void playDeleteTone();
void playErrorTone();
void playChangePWDTone();
void playDownloadTone();

// Etc



#endif