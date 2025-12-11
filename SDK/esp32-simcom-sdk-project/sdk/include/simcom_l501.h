#ifndef SIMCOM_L501_H
#define SIMCOM_L501_H

#include <Arduino.h>

// Function prototypes for SIMCom L501 SDK
void simcom_l501_init(HardwareSerial &serial);
void simcom_l501_send_command(const char *command);
String simcom_l501_receive_response();

#endif // SIMCOM_L501_H