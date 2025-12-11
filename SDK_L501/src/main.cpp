#include <Arduino.h>
#include <AIoT_L501.h>

// Khai báo đối tượng AIoT_L501 sử dụng UART2 (GPIO17: TX, GPIO16: RX)
AIoT_L501 aiot(Serial2, 115200);

void setup() {
    Serial.begin(115200); // Serial debug
    aiot.begin();
    delay(1000); // Đợi module khởi động

    String resp;
    if (aiot.sendAT("AT", resp)) {
        Serial.println("SIMCom L501 OK:");
        Serial.println(resp);
    } else {
        Serial.println("SIMCom L501 ERROR:");
        Serial.println(resp);
    }
}

void loop() {
    // Có thể gửi thêm lệnh AT ở đây nếu muốn
}