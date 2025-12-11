#include "simcom_l501.h"
#include <HardwareSerial.h>

HardwareSerial simcomSerial(1); // Use UART1 for SIMCom L501

void simcom_init(long baudrate) {
    simcomSerial.begin(baudrate, SERIAL_8N1, 16, 17); // RX on GPIO 16, TX on GPIO 17
}

bool simcom_send_command(const char* command, char* response, size_t response_size, unsigned long timeout) {
    simcomSerial.println(command);
    unsigned long start_time = millis();
    size_t index = 0;

    while (millis() - start_time < timeout) {
        if (simcomSerial.available()) {
            char c = simcomSerial.read();
            if (index < response_size - 1) {
                response[index++] = c;
            }
            if (c == '\n') {
                response[index] = '\0'; // Null-terminate the response
                return true;
            }
        }
    }
    return false; // Timeout
}

bool simcom_check_response(const char* expected_response, const char* actual_response) {
    return strcmp(expected_response, actual_response) == 0;
}