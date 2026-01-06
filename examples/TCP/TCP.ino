#include <Arduino.h>
#include <AIoT_L501.h>


#define L501_RX 16
#define L501_TX 17

AIoT_L501 aiot(Serial2, 115200, L501_RX, L501_TX);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    delay(5000);

    Serial.println("=== TEST TCP SEND & RECEIVE ===");

    aiot.begin();

    // 1. Init module + SIM + network
    if (!aiot.init(10000) || !aiot.ensureNetwork(3, 3000)) {
        Serial.println("[-] Module/SIM/network error!");
        return;
    }
    Serial.println("[+] Module & SIM OK");

    // 2. Internet 4G
    if (!aiot.connectInternet4G("v-internet")) {
        Serial.println("[-] Internet 4G error!");
        return;
    }
    Serial.println("[+] Internet 4G OK");

    // 3. NETOPEN
    if (!aiot.netOpen()) {
        Serial.println("[-] NETOPEN error!");
        return;
    }
    Serial.println("[+] NETOPEN OK");
    
    // 4. TCP connect
    int socketId = 1;
    if (!aiot.tcpConnect(socketId, "tcpbin.com", 4242)) {
        Serial.println("[-] TCP connect error!");
        return;
    }
    Serial.println("[+] TCP connected!");

    delay(500);

    // 5. TCP send
    String data = " viet dang test L501\r\n";
    if (!aiot.tcpSend(socketId, data)) {
        Serial.println("[-] TCP send FAIL");
        aiot.tcpClose(socketId);
        return;
    }
    Serial.print("[+] TCP send OK: ");
    Serial.print(data);

    // 6. CHỜ SERVER TRẢ DATA
    delay(1000);

    // 7. KIỂM TRA & NHẬN TCP
    
        String rx = aiot.tcpReceive(socketId, 128);
        Serial.print("[+] TCP received: ");
        Serial.println(rx);

    //8. Đóng TCP
    if (aiot.tcpClose(socketId)) {
        Serial.println("[+] TCP closed");
    } else {
        Serial.println("[-] TCP close FAIL");
    
    }
    //aiot.cleanStart();
}

void loop() {
    // Test 1 lần trong setup
}
