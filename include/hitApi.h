// hitApi.h
#ifndef HIT_API_H
#define HIT_API_H

void getStatusAndPinDevice();
bool getAllRfid();
void loginDevice(String macAddress);
bool getNewPin();
void detectPinSend();
bool sendHistoryRfid(String rfid);

#endif