#include <Arduino.h>

void handleSerialInput(){

if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil('\n');
    if (command == "/command")
    {
      Serial.println("1. Restart ESP");
      Serial.println("2. Reset EPROOM ESP");
      Serial.println("3. Open Wifi WebClient");
      Serial.println("4. Close Wifi WebClient");
      String select = Serial.readStringUntil('\n');
      if (select == "1")
      {
        Serial.println("Restarting...");
        ESP.restart();
      }else if(select == "2"){
        Serial.println("Reseting EPROOM...");
        delay(2000);
        ESP.restart();
      }
    }
  }

}
