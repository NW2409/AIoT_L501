#include <Arduino.h>
#include <AIoT_L501.h>

#define L501_RX 16
#define L501_TX 17

AIoT_L501 aiot(Serial2, 115200, L501_RX, L501_TX);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("=== L501 PING TEST ===");

    aiot.begin();

    // Khởi tạo module và kiểm tra mạng
    if (!aiot.init(10000) || !aiot.ensureNetwork(3, 3000)) {
        Serial.println("[-] Module hoặc mạng lỗi!");
        return;
    }
    Serial.println("[+] Module & SIM OK");

    // Kết nối Internet 4G (APN tuỳ nhà mạng, ví dụ: "v-internet" cho Viettel)
    if (!aiot.connectInternet4G("v-internet")) {
        Serial.println("[-] Không kết nối được Internet 4G!");
        return;
    }

    // Ping đến địa chỉ
    Serial.println("[*] Đang ping ...");
    String pingResult = aiot.ping("8.8.8.8", 5, 32, 1);
    Serial.println("[Ping Result]");
    Serial.println(pingResult);
}

void loop() {
    // Không làm gì
}