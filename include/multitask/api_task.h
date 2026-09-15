// api_task.h
#ifndef API_TASK_H
#define API_TASK_H

// void getStatusAndPinDevice();
void getStatusTask(void *pvParameters);
void versionTask(void *pvParameters);

void loginDevice(String macAddress);

bool getAllRfid();
bool getNewPin();
void detectPinSend();
void sendDeviceVersion();



#endif
