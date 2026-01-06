#include <Arduino.h>
#include <AIoT_L501.h>

#define L501_RX 16
#define L501_TX 17

AIoT_L501 aiot(Serial2, 115200, L501_RX, L501_TX);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("=== DEMO 1: BRING-UP & INIT CƠ BẢN ===");

    aiot.begin();

    unsigned long t0 = millis();
    bool ok = aiot.init(10000);
    unsigned long elapsed = millis() - t0;

    // AT OK
    Serial.print("[AT OK] ");
    Serial.println(ok ? "YES" : "NO");

    // SIM READY
    Serial.print("[SIM READY] ");
    Serial.println(aiot.isSimReady() ? "YES" : "NO");

    // CSQ (RSSI)
    int rssi = aiot.getSignalQuality();
    Serial.print("[CSQ RSSI] ");
    Serial.println(rssi);

    // Network Registered
    bool netReg = aiot.isNetworkRegistered();
    Serial.print("[Network Registered] ");
    Serial.println(netReg ? "YES" : "NO");

    // Kiểm tra Internet
    Serial.print("[Internet] ");
    String ip = aiot.getIPAddress();
    if (ip.length() > 0 && ip != "0.0.0.0") {
        Serial.println("CONNECTED (" + ip + ")");
    } else {
        Serial.println("NOT CONNECTED");
    }

    // Pass/Fail
    if (ok && elapsed < 10000 && aiot.isSimReady() && netReg) {
        Serial.printf("[PASS] init() hoàn tất trong %lu ms\n", elapsed);
    } else {
        Serial.printf("[-] INIT FAIL (thời gian: %lu ms)\n", elapsed);
    }
}

void loop() {
    // Không làm gì
}