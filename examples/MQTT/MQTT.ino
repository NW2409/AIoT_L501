#include <Arduino.h>
#include <AIoT_L501.h>

#define LED_PIN 2
#define L501_RX 16
#define L501_TX 17

AIoT_L501 aiot(Serial2, 115200, L501_RX, L501_TX);
static const char* APN = "v-internet";

unsigned long t0, t1;

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== KHOI CHAY KET NOI MQTT THU CONG ===");

    // Đo thời gian cho từng bước (bỏ đo heap)
    t0 = millis();
    aiot.begin();
    t1 = millis();
    Serial.printf("[TIME] begin: %lu ms\n", t1-t0);

    aiot.mqttStop();
    delay(1000);

    t0 = millis();
    bool ok = aiot.init(10000);
    t1 = millis();
    Serial.printf("[TIME] init: %lu ms\n", t1-t0);

    t0 = millis();
    ok &= aiot.ensureNetwork(3, 3000);
    t1 = millis();
    Serial.printf("[TIME] ensureNetwork: %lu ms\n", t1-t0);

    if (!ok) {
        Serial.println("[-] Loi khoi tao hoac mang khong san sang");
        return;
    }

    t0 = millis();
    bool inetOk = aiot.connectInternet4G(APN);
    t1 = millis();
    Serial.printf("[TIME] connectInternet4G: %lu ms\n", t1-t0);

    t0 = millis();
    bool mqttOk = aiot.mqttConfig("L501Device");
    t1 = millis();
    Serial.printf("[TIME] mqttConfig: %lu ms\n", t1-t0);

    t0 = millis();
    bool mip = aiot.mqttSetServer("broker.emqx.io", 1883, 3);
    t1 = millis();
    Serial.printf("[TIME] mqttSetServer: %lu ms\n", t1-t0);

    delay(2000);

    bool connected = false;
    t0 = millis();
    for (int i = 0; i < 3; i++) {
        if (aiot.mqttConnect(0, 60)) {
            connected = true;
            break;
        }
        delay(3000);
    }
    t1 = millis();
    Serial.printf("[TIME] mqttConnect: %lu ms\n", t1-t0);

    if (!connected) {
        Serial.println("[-] MCONNECT FAIL sau 3 lan - Kiem tra ID hoac Broker");
        return;
    }

    String topic = "testsim";
    aiot.mqttPublish(topic, "L501 KET NOI THANH CONG");
    aiot.mqttSubscribe(topic);

    Serial.println("=== BAT DAU NHAN TIN NHAN TRONG LOOP ===");
}


void loop() {
    unsigned long loopStart = micros();

    // Nhận tin
    String msg = aiot.mqttReceive(500);
    if (msg.length() > 0) {
        Serial.println("[RX]: " + msg);
        if (msg.indexOf("testsim") != -1) {
            int lastComma = msg.lastIndexOf(',');
            String payload = msg.substring(lastComma + 1);
            payload.trim();
            payload.replace("\"", "");
            if (payload.equalsIgnoreCase("ON")) {
                digitalWrite(LED_PIN, HIGH);
                Serial.println("[LED] ON");
                bool ok = aiot.mqttPublish("testsim", "Đã bật");
            } else if (payload.equalsIgnoreCase("OFF")) {
                digitalWrite(LED_PIN, LOW);
                Serial.println("[LED] OFF");
                bool ok = aiot.mqttPublish("testsim", "Đã tắt");
            }
        }
    }

}