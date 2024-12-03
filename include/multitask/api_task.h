// api_data.h
#ifndef API_TASK_H
#define API_TASK_H

// void GetStatusDevice();
void getStatusTask(void *pvParameters);

void loginDevice(String macAddress);

bool getAllRfid();
bool getNewPin();

void detectPinSend();



#endif
