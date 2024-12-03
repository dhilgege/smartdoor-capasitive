#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

void setupWifi();
void checkConnection();
// void hardResetSettings();
void WiFiReconnectTask(void *parameter);
void openWebPortal();
void checkButton();


#endif
