// api_data.h
#ifndef HIT_API
#define HIT_API

void getStatusAndPinDevice();
bool getAllRfid();
void loginDevice(String macAddress);
bool getNewPin();
void detectPinSend();
bool sendHistoryRfid(String rfid);

#endif