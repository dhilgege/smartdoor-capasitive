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
#include "config/config.h"
#include "hardware.h"
#include "wifi_manager.h"
#include "otaHandleMain.h"
#include "hitApi.h"

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

Keypad kpd = Keypad(makeKeymap(keys), KEYPAD_ROW_PINS, KEYPAD_COL_PINS, KEYPAD_ROWS, KEYPAD_COLS);

unsigned long loopCount;
unsigned long startTime;
String msg;

String inputPin = "";

void handleKeypad()
{
    char key = kpd.getKey();

    if (key)
    {
        if (key == 'D')
        {
            inputPin = "";
            Serial.println("Input direset.");
            playDeleteTone();
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
                    ActivateDoorTimer(3000);
                    playSuccessTone();
                }
                else if (inputPin == "##01##")
                {
                    playResetTone(200);
                    Serial.println("Restart Device");
                    delay(500);
                    ESP.restart();
                }
                else if (inputPin == "##02##")
                {
                    playResetTone(200);
                    Serial.println("Reset EEPROOM ESP");
                    delay(500);
                    resetEpproomWifi();
                }
                else if (inputPin == "##03##")
                {
                    playResetTone(200);
                    Serial.println("Resseting Wifi & Setup Wifi");
                    delay(500);
                    resetEpproomWifi();
                    setupWifi();

                }
                else if (inputPin == "##04##")
                {
                    playResetTone(200);
                    Serial.println("Force OTA calling");
                    delay(500);
                    enterOtaMode();
                    setupWifi();

                }
                else if (inputPin == "##05##")
                {
                    playResetTone(200);
                    Serial.println("Request get RFID");
                    delay(500);
                    getAllRfid();
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
        if (isOtaMode)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        if (doorTimerActive && millis() - doorTimerStart >= doorTimerDuration)
        {
            ActivateDoor(false);
            Serial.println("Door Closed");
            doorTimerActive = false;
        }

        handleKeypad();

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}