#include <Arduino.h>



extern int LED_Red;
extern int LED_Green;
extern int LED_Yellow;
extern int DoorPin;
extern int resetpin;
extern int detectPin;
extern int BUZZER_PIN;
extern int EXT_LED_RED;
extern int EXT_LED_GREEN;
extern int EXT_LED_BLUE;

void pinSetup(){

    // pin LED bawaan
    pinMode(LED_Red, OUTPUT);
    pinMode(LED_Green, OUTPUT);
    pinMode(LED_Yellow, OUTPUT);

    //pin LED eksternal
    pinMode(EXT_LED_RED, OUTPUT);
    pinMode(EXT_LED_GREEN, OUTPUT);
    pinMode(EXT_LED_BLUE, OUTPUT);

            digitalWrite(EXT_LED_RED, HIGH);

    pinMode(DoorPin, OUTPUT);        // pin Relay
    digitalWrite(DoorPin, LOW);
    pinMode(resetpin, INPUT_PULLUP);  // pin Relay
    pinMode(detectPin, INPUT_PULLUP); // pin Relay
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


void ActivateDoorTimer(int time) {
    Serial.println("Door Opened"+ time);
unsigned long doorOpenTime = 0;  // Variabel untuk menyimpan waktu saat pintu dibuka
bool doorOpened = false;         // Status pintu terbuka atau tertutup
    if (!doorOpened) {  // Jika pintu belum terbuka, buka pintu
        digitalWrite(DoorPin, HIGH);
        Serial.println("Door Opened");
        doorOpenTime = millis(); // Catat waktu saat pintu dibuka
        doorOpened = true;       // Ubah status pintu menjadi terbuka
    } else {
        // Cek apakah durasi membuka pintu telah tercapai
        if (millis() - doorOpenTime >= time) {
            digitalWrite(DoorPin, LOW);
            Serial.println("Door Closed");
            doorOpened = false; // Reset status pintu
        }
    }
}


void playKeypadTone()
{
    tone(BUZZER_PIN, 1000, 100); // Nada 1000 Hz selama 100 ms
}

void playResetTone(int time)
{
    tone(BUZZER_PIN, 3000, time); // Nada 3000 Hz selama 100 ms
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