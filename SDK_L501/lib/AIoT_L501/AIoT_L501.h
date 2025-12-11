#pragma once
#include <Arduino.h>

class AIoT_L501 {
public:
    AIoT_L501(HardwareSerial &serial, uint32_t baud = 115200);
    void begin();
    bool sendAT(const char *cmd, String &response, uint32_t timeout = 2000);
    bool isSimReady(); // Kiểm tra trạng thái SIM
    bool isNetworkRegistered();
    String getIMEI();
    bool sendSMS(const String &phone, const String &message);
    bool call(const String &phone);
    bool hangUp();
    bool answerCall();
    int getSignalQuality();
    String getModuleInfo();
    // MQTT
    bool mqttConfig(const String &clientId, const String &username = "", const String &password = "");
    bool mqttSetServer(const String &address, int port, int version = 4);
    bool mqttConnect(int cleanSession = 1, int keepalive = 60);
    bool mqttPublish(const String &topic, const String &payload, int qos = 0, int retain = 0);
    bool mqttSubscribe(const String &topic, int qos = 0);
    bool mqttUnsubscribe(const String &topic);
    bool mqttDisconnect();
    bool mqttClose();
    int  mqttStatus();
    bool attachGPRS(const String &apn, const String &user = "", const String &pass = "");
    bool activatePDP(int cid = 1);
    bool deactivatePDP(int cid = 1);
private:
    HardwareSerial &serial_;
    uint32_t baud_;
};