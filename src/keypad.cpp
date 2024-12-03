#include <Arduino.h>
/* @file MultiKey.ino
|| @version 1.0
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
|| @description
|| | The latest version, 3.0, of the keypad library supports up to 10
|| | active keys all being pressed at the same time. This sketch is an
|| | example of how you can get multiple key presses from a keypad or
|| | keyboard.
|| #
*/

#include <Keypad.h>

#include "hardware.h"
#include "wifi_manager.h"

const byte ROWS = 4; // four rows
const byte COLS = 4; // three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte colPins[COLS] = {17, 16, 15, 14};
byte rowPins[ROWS] = {6, 5, 2, 1};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

unsigned long loopCount;
unsigned long startTime;
String msg;

extern String StoredDoorPin;
String inputPin = "";

void handleKeypad()
{
    char key = kpd.getKey();

    if (key)
    {
        if (key == '*')
        {
            inputPin = "";
            Serial.println("Input direset.");
        }
        else
        {
            inputPin += key;
            Serial.print("Input: ");
            Serial.println(inputPin);
            playKeypadTone();

            // Jika sudah 6 digit, periksa PIN
            if (inputPin.length() == 6)
            {
                if (inputPin == StoredDoorPin)
                {
                    Serial.println("PIN benar! Akses diizinkan.");
                    playSuccessTone();
                    ActivateDoorTimer(3000);
                }
                else if (inputPin == "##01##")
                {
                    Serial.println("Restart Device");
                    playResetTone(1500);
                    delay(20);
                    ESP.restart();
                }
                else
                {
                    playErrorTone();
                    Serial.println("PIN salah! Coba lagi.");
                }
                inputPin = ""; // Reset input
            }
        }
    }
}

void KeypadTask(void *pvParameters)
{
    for (;;)
    {
        handleKeypad();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}