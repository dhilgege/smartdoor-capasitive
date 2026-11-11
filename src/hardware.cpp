#include <Arduino.h>
#include <WiFi.h>
#include "config/config.h"
#include "hardware.h"
#include "ethernet_manager.h"

void pinSetup(){
    initErrorIndicator();
    initNetworkIndicator();
    // pin LED bawaan
    pinMode(LED_Red, OUTPUT);
    pinMode(LED_Green, OUTPUT);
    pinMode(LED_Yellow, OUTPUT);

    // Matikan semua LED di awal
    digitalWrite(LED_Red, LOW);
    digitalWrite(LED_Green, LOW);
    digitalWrite(LED_Yellow, LOW);

    pinMode(DoorPin, OUTPUT);        // pin Relay
    digitalWrite(DoorPin, LOW);
    pinMode(resetpin, INPUT_PULLUP);  // pin Relay
    pinMode(detectPin, INPUT_PULLUP); // pin Relay

    // Buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Tombol exit (harus setelah DoorPin & BUZZER_PIN siap)
    initExitButton();
}

bool isSystemError = false;
TaskHandle_t ErrorTaskHandle = NULL;

void setErrorState(bool state) {
    if (isSystemError != state) {
        isSystemError = state;
        if (!state) {
            // Matikan LED dan Buzzer jika error selesai
            digitalWrite(LED_Red, LOW);
            noTone(BUZZER_PIN);
        }
    }
}

void ErrorIndicatorTask(void *parameter) {
    for(;;) {
        if (isSystemError) {
            // Blink & Beep 5x
            for(int i=0; i<5; i++) {
                if (!isSystemError) break; // Keluar jika error tiba2 sembuh
                digitalWrite(LED_Red, HIGH);
                tone(BUZZER_PIN, 1000); 
                vTaskDelay(200 / portTICK_PERIOD_MS);
                
                digitalWrite(LED_Red, LOW);
                noTone(BUZZER_PIN);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
            
            // Setelah 5x, lampu merah diam (ON), buzzer mati selama 30 detik
            if (isSystemError) {
                digitalWrite(LED_Red, HIGH);
                noTone(BUZZER_PIN);
                // Tunggu 30 detik, tapi pecah delaynya agar bisa cepat responsif jika error sembuh
                for(int i=0; i<300; i++) {
                    if (!isSystemError) {
                        digitalWrite(LED_Red, LOW);
                        break;
                    }
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
            }
        } else {
            // Jika tidak error, istirahat dan cek lagi
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

void initErrorIndicator() {
    xTaskCreatePinnedToCore(
        ErrorIndicatorTask,
        "ErrorIndicatorTask",
        2048,
        NULL,
        1,
        &ErrorTaskHandle,
        1
    );
}

void NetworkIndicatorTask(void *parameter) {
    bool wifiBlinkState = false;
    for (;;) {
        if (isEthernetConnected()) {
            // LAN aktif -> LED Kuning menyala mantap (Solid ON)
            digitalWrite(LED_Yellow, HIGH);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else if (WiFi.status() == WL_CONNECTED) {
            // WiFi aktif (Backup) -> LED Kuning berkedip (500ms ON / 500ms OFF)
            wifiBlinkState = !wifiBlinkState;
            digitalWrite(LED_Yellow, wifiBlinkState ? HIGH : LOW);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else {
            // Tidak ada koneksi (Offline / Disconnected) -> LED Kuning MATI
            digitalWrite(LED_Yellow, LOW);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

void initNetworkIndicator() {
    xTaskCreatePinnedToCore(
        NetworkIndicatorTask,
        "NetIndicatorTask",
        2048,
        NULL,
        1,
        NULL,
        0 // Core 0 (bersama network task)
    );
}

void blinkLedRed(bool state)
{
    // Function ini sekarang tidak disarankan untuk digunakan langsung
    // karena sudah ada setErrorState(true/false)
    if (state) setErrorState(true);
    else setErrorState(false);
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

// =============================================================================
// TOMBOL EXIT (PUSH ON / NO)
// Kaki 1 -> EXIT_BTN_PIN (GPIO 13, J3 pin 6), kaki 2 -> GND (J3 pin 7)
// Ditekan = LOW (INPUT_PULLUP). Berjalan di task sendiri supaya tetap responsif
// walaupun loop() sedang menunggu HTTP/delay.
// =============================================================================
static const unsigned long EXIT_BTN_DEBOUNCE_MS = 50;

static void ExitButtonTask(void *parameter) {
    bool lastRead   = digitalRead(EXIT_BTN_PIN);
    bool lastStable = lastRead; // awalnya ikut kondisi pin saat ini (cegah trigger palsu saat boot)
    unsigned long lastChange = millis();

    for (;;) {
        bool reading = digitalRead(EXIT_BTN_PIN);

        if (reading != lastRead) {
            lastRead = reading;
            lastChange = millis();
        }

        if ((millis() - lastChange) >= EXIT_BTN_DEBOUNCE_MS && reading != lastStable) {
            lastStable = reading;

            if (lastStable == LOW && !isOtaMode) { // tombol baru ditekan
                Serial.println("[EXIT] Tombol exit ditekan, pintu dibuka");
                ActivateDoorTimer(DEFAULT_DOOR_TIMER_MS);
                playKeypadTone();
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void initExitButton() {
    pinMode(EXIT_BTN_PIN, INPUT_PULLUP);
    xTaskCreatePinnedToCore(
        ExitButtonTask,
        "ExitButtonTask",
        3072,
        NULL,
        2,
        NULL,
        1 // Core 1
    );
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
        setErrorState(true);
        Serial.println("\nError LED RED diaktifkan");
    }
    else if (state == "GREEN")
    {
        digitalWrite(LED_Green, HIGH);
    }
    else if (state == "BLUE")
    {
        digitalWrite(LED_Yellow, HIGH); // Pakai YELLOW karena BLUE tidak ada di skematik
    }
}

void TurnOFFLed(String state)
{
    if (state == "RED")
    {
        setErrorState(false);
    }
    else if (state == "GREEN")
    {
        digitalWrite(LED_Green, LOW);
    }
    else if (state == "BLUE")
    {
        digitalWrite(LED_Yellow, LOW);
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