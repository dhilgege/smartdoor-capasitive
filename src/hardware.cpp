#include <Arduino.h>
#include "config/config.h"
#include "hardware.h"



void pinSetup(){

    // pin LED bawaan
    pinMode(LED_Red, OUTPUT);
    pinMode(LED_Green, OUTPUT);
    pinMode(LED_Yellow, OUTPUT);

    //pin LED eksternal
    pinMode(EXT_LED_RED, OUTPUT);
#if (ACTIVE_KEYPAD_TYPE != KEYPAD_TYPE_I2C_TOUCH_4X3)
    // GPIO 17 digunakan sebagai IRQ keypad I2C saat mode capacitive touch aktif.
    // Hindari override ke OUTPUT agar tidak mengganggu sinyal interrupt.
    pinMode(EXT_LED_GREEN, OUTPUT);
    pinMode(EXT_LED_BLUE, OUTPUT);
#endif

            digitalWrite(EXT_LED_RED, HIGH);

    pinMode(DoorPin, OUTPUT);        // pin Relay
    digitalWrite(DoorPin, LOW);
    pinMode(resetpin, INPUT_PULLUP);  // pin Relay
    pinMode(detectPin, INPUT_PULLUP); // pin Relay

    // Buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void blinkLedRed(bool state)
{
    static unsigned long previousMillis = 0;
    const long interval = 500;
    if (state)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            digitalWrite(LED_Red, !digitalRead(LED_Red));
        }
    }
    else
    {
        digitalWrite(LED_Red, LOW);
    }
}

void ActivateDoor(bool state)
{
    if (state)
    {
        digitalWrite(DoorPin, HIGH);
    }
    else
    {
        digitalWrite(DoorPin, LOW);
    }
}

// void ActivateDoorTimer(int time)
// {
//     digitalWrite(DoorPin, HIGH);
//     Serial.println("Door Opened");
//     delay(time);
//     digitalWrite(DoorPin, LOW);
//     Serial.println("Door Closed");
// }


void ActivateDoorTimer(int duration) {
    // digitalWrite(DoorPin, HIGH);
    ActivateDoor(true); // Menggunakan fungsi ActivateDoor
    Serial.println("Door Opened");
    doorTimerStart = millis();
    doorTimerDuration = duration;
    doorTimerActive = true;
}



void playKeypadTone()
{
    tone(BUZZER_PIN, 1000, 100); // Nada 1000 Hz selama 100 ms
}

// Nada unik per tombol (tangga nada musik) — dari kode test keypad yang berhasil
void playKeypadToneForKey(char key)
{
    uint16_t freq;
    switch (key) {
        case '1': freq = 262; break;  // C4
        case '2': freq = 294; break;  // D4
        case '3': freq = 330; break;  // E4
        case '4': freq = 349; break;  // F4
        case '5': freq = 392; break;  // G4
        case '6': freq = 440; break;  // A4
        case '7': freq = 494; break;  // B4
        case '8': freq = 523; break;  // C5
        case '9': freq = 587; break;  // D5
        case '0': freq = 220; break;  // A3
        case 'A': freq = 659; break;  // E5
        case 'B': freq = 698; break;  // F5
        default:  freq = 1000; break; // fallback
    }
    tone(BUZZER_PIN, freq, 100); // Nada 100 ms
}

void playResetTone(int time)
{
    tone(BUZZER_PIN, 3000, time); // Nada 3000 Hz selama 100 ms
    delay(500);
    tone(BUZZER_PIN, 3000, time); // Nada 3000 Hz selama 100 ms
    delay(500);
    tone(BUZZER_PIN, 3000, time); // Nada 3000 Hz selama 100 ms
    delay(500);
}

void playSuccessTone()
{
    tone(BUZZER_PIN, 2000, 300); // Nada 2000 Hz selama 300 ms
    delay(300);
    tone(BUZZER_PIN, 2500, 300); // Nada 2500 Hz selama 300 ms
    delay(300);
}

void playErrorTone()
{
    tone(BUZZER_PIN, 500, 500); // Nada 500 Hz selama 500 ms
    delay(500);
}
void playDeleteTone()
{
    tone(BUZZER_PIN, 400, 500); // Nada 500 Hz selama 500 ms
    delay(500);
}

void playDownloadTone()
{
    tone(BUZZER_PIN, 800, 100); // Nada 500 Hz selama 500 ms
    delay(500);
}


void playChangePWDTone()
{
    // Beep panjang 1 kali
    tone(BUZZER_PIN, 1000, 1000); // Nada 1000 Hz selama 1000 ms
    delay(1000);                  // Tunggu selama 1000 ms

    // Beep pendek 2 kali
    tone(BUZZER_PIN, 1000, 200); // Nada 1000 Hz selama 200 ms
    delay(200);                  // Tunggu selama 200 ms
    tone(BUZZER_PIN, 1000, 200); // Nada 1000 Hz selama 200 ms
    delay(200);                  // Tunggu selama 200 ms
}

void TurnOnLed(String state)
{
    if (state == "RED")
    {
        digitalWrite(EXT_LED_RED, HIGH);
        Serial.println("\nLED RED nyala");
        Serial.println(digitalRead(EXT_LED_RED));

    }
    else if (state == "GREEN")
    {
        digitalWrite(EXT_LED_GREEN, HIGH);
    }
    else if (state == "BLUE")
    {
        digitalWrite(EXT_LED_BLUE, HIGH);
    }
}

void TurnOFFLed(String state)
{
    if (state == "RED")
    {
        digitalWrite(EXT_LED_RED, LOW);
    }
    else if (state == "GREEN")
    {
        digitalWrite(EXT_LED_GREEN, LOW);
    }
    else if (state == "BLUE")
    {
        digitalWrite(EXT_LED_BLUE, LOW);
    }
}

// unsigned long lastToneTime = 0;
// int toneStep = 0;
// bool isPlayingTone = false;

// void playKeypadTone() {
//     if (!isPlayingTone) {
//         tone(BUZZER_PIN, 1000, 100); // Nada 1000 Hz selama 100 ms
//         isPlayingTone = true;
//         lastToneTime = millis();
//     }
//     if (millis() - lastToneTime >= 100) {
//         noTone(BUZZER_PIN);
//         isPlayingTone = false;
//     }
// }

// void playSuccessTone() {
//     if (!isPlayingTone) {
//         switch (toneStep) {
//             case 0:
//                 tone(BUZZER_PIN, 2000, 300); // Nada 2000 Hz selama 300 ms
//                 lastToneTime = millis();
//                 toneStep = 1;
//                 isPlayingTone = true;
//                 break;
//             case 1:
//                 if (millis() - lastToneTime >= 300) {
//                     tone(BUZZER_PIN, 2500, 300); // Nada 2500 Hz selama 300 ms
//                     lastToneTime = millis();
//                     toneStep = 2;
//                 }
//                 break;
//             case 2:
//                 if (millis() - lastToneTime >= 300) {
//                     noTone(BUZZER_PIN);
//                     toneStep = 0;
//                     isPlayingTone = false;
//                 }
//                 break;
//         }
//     }
// }

// void playErrorTone() {
//     if (!isPlayingTone) {
//         tone(BUZZER_PIN, 500, 500); // Nada 500 Hz selama 500 ms
//         lastToneTime = millis();
//         isPlayingTone = true;
//     }
//     if (millis() - lastToneTime >= 500) {
//         noTone(BUZZER_PIN);
//         isPlayingTone = false;
//     }
// }

// void playChangePWDTone() {
//     if (!isPlayingTone) {
//         switch (toneStep) {
//             case 0:
//                 tone(BUZZER_PIN, 1000, 1000); // Nada 1000 Hz selama 1000 ms
//                 lastToneTime = millis();
//                 toneStep = 1;
//                 isPlayingTone = true;
//                 break;
//             case 1:
//                 if (millis() - lastToneTime >= 1000) {
//                     tone(BUZZER_PIN, 1000, 200); // Nada 1000 Hz selama 200 ms
//                     lastToneTime = millis();
//                     toneStep = 2;
//                 }
//                 break;
//             case 2:
//                 if (millis() - lastToneTime >= 200) {
//                     noTone(BUZZER_PIN);
//                     lastToneTime = millis();
//                     toneStep = 3;
//                 }
//                 break;
//             case 3:
//                 if (millis() - lastToneTime >= 200) {
//                     tone(BUZZER_PIN, 1000, 200); // Nada 1000 Hz selama 200 ms
//                     lastToneTime = millis();
//                     toneStep = 4;
//                 }
//                 break;
//             case 4:
//                 if (millis() - lastToneTime >= 200) {
//                     noTone(BUZZER_PIN);
//                     toneStep = 0;
//                     isPlayingTone = false;
//                 }
//                 break;
//         }
//     }
// }