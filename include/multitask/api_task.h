// api_data.h
#ifndef API_TASK_H
#define API_TASK_H

// void GetStatusDevice();
void getStatusTask(void *pvParameters);

void loginDevice(String macAddress);

void getAllRfid();
void getNewPin();

void detectPinSend();



#endif
