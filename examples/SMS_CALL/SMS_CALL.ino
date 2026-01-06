#include <Arduino.h>
#include <AIoT_L501.h>

#define L501_RX 16
#define L501_TX 17

AIoT_L501 aiot(Serial2, 115200, L501_RX, L501_TX);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    delay(1000);
    Serial.println("=== TEST SEND SMS & CALL ===");

    aiot.begin();

    // Khởi tạo module và mạng
    if (!aiot.init(10000) || !aiot.ensureNetwork(3, 3000)) {
        Serial.println("[-] Module or network init FAIL");
        return;
    }
    Serial.println("[+] Module & SIM OK");

    // Gửi SMS
    String phone = "+84973377546";
    String message = "Xin chao tu L501!";
    bool ok = aiot.sendSMS(phone, message);
    if (ok) {
        Serial.println("[+] SMS sent OK!");
    } else {
        Serial.println("[-] SMS send FAIL!");
    }

    // Thêm delay để module xử lý xong SMS
    delay(5000);

    // Gọi điện
    Serial.println("[DBG] Dang goi dien thoai...");
    ok = aiot.call(phone);
    if (ok) {
        Serial.println("[+] Call started!");
        delay(10000); // Đợi 10 giây rồi ngắt cuộc gọi
        Serial.println("[+] Call ended!");
    } else {
        Serial.println("[-] Call FAIL!");
    }
}

void loop() {
    // Không làm gì
}