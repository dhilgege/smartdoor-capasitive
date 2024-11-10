#include <WiFi.h>
#include <WiFiManager.h>
#include "wifi_manager.h"

extern char *customHostname;
extern char *ssidwifimanager;
extern char *passwordwifimanager;
extern const int resetpin;

WiFiManager wm;

void setupWiFi()
{
  // wifi manager
  WiFi.setHostname(customHostname);
  bool res;
  res = wm.autoConnect(ssidwifimanager, passwordwifimanager); // password protected ap
  if (!res)
  {
    Serial.println("Failed to connect");
    ESP.restart();
  }
  else
  {
    Serial.println("terkoneksi ke wifi :)");
  }
}

void checkConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected, reconnecting...");
    wm.autoConnect(ssidwifimanager, passwordwifimanager);
  }
}

// void hardResetSettings()
// {

//   // resetconfig
//   if (digitalRead(resetpin) == LOW)
//   {
//     delay(50);
//     if (digitalRead(resetpin) == LOW)
//     {
//       Serial.println("Button Pressed");
//       delay(3000); // reset delay hold
//       if (digitalRead(resetpin) == LOW)
//       {
//         Serial.println("Button Held");
//         Serial.println("Erasing Config, restarting");
//         wm.resetSettings();
//         ESP.restart();
//       }
//     }
//   }
// }



void WiFiTask(void *pvParameters)
{
  for (;;)
  {
    checkConnection();
    delay(1000);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}