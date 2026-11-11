#ifndef ETHERNET_MANAGER_H
#define ETHERNET_MANAGER_H

#include <Arduino.h>

void ethernetSetup();
bool isEthernetConnected();
bool isEthernetLinkUp();
IPAddress getEthernetIP();
String getEthernetMac();

#endif // ETHERNET_MANAGER_H
