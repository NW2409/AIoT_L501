#include <Arduino.h>
#include <HardwareSerial.h>
#include "simcom_l501.h"

#define UART_NUM UART_NUM_1
#define TX_PIN 17
#define RX_PIN 16

HardwareSerial simcomSerial(UART_NUM);

void setup() {
  Serial.begin(115200);
  simcomSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Initialize the SIMCom L501 module
  if (simcom_init() == 0) {
    Serial.println("SIMCom L501 initialized successfully.");
  } else {
    Serial.println("Failed to initialize SIMCom L501.");
  }

  // Example of sending an AT command
  const char* command = "AT\r";
  simcom_send_command(command);
  
  // Wait for a response
  String response = simcom_receive_response();
  Serial.println("Response: " + response);
}

void loop() {
  // Main loop can be used for further communication or processing
}