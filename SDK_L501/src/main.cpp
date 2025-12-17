#include <Arduino.h>
#include <AIoT_L501.h>

// Khai báo đối tượng AIoT_L501 sử dụng UART2 (GPIO17: TX, GPIO16: RX)
AIoT_L501 aiot(Serial2, 115200);

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Gọi hàm init() - sẽ in logo AIoT và kiểm tra AT, AT+CSQ, AT+CPIN?
    if (aiot.init(10000)) {
        Serial.println("Module sẵn sàng!");
    } else {
        Serial.println("Khởi tạo module thất bại!");
    }
}

void loop() {
    // Code chính ở đây
}