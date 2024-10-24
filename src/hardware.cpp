#include <Arduino.h>


extern int LED_PIN;
extern int DoorPin;
extern int resetpin;
extern int detectPin;
extern int BUZZER_PIN;

void pinSetup()
{
    pinMode(LED_PIN, OUTPUT);         // pin LED bawaan
    pinMode(DoorPin, OUTPUT);        // pin Relay
    pinMode(resetpin, INPUT_PULLUP);  // pin Relay
    pinMode(detectPin, INPUT_PULLUP); // pin Relay
}

void blinkLED(bool state)
{
    static unsigned long previousMillis = 0;
    const long interval = 500;
    if (state)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
    }
    else
    {
        digitalWrite(LED_PIN, LOW);
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

void ActivateDoorTimer(int time)
{
    digitalWrite(DoorPin, HIGH);
    Serial.println("Door Opened");
    delay(time);
    digitalWrite(DoorPin, LOW);
    Serial.println("Door Closed");
}

void playKeypadTone()
{
    tone(BUZZER_PIN, 1000, 100); // Nada 1000 Hz selama 100 ms
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