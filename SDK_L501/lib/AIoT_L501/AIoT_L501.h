#pragma once
#include <Arduino.h>

class AIoT_L501 {
public:
    AIoT_L501(HardwareSerial &serial, uint32_t baud = 115200); // Khởi tạo đối tượng với Serial và baudrate
    void begin(); // Khởi động Serial
    bool sendAT(const char *cmd, String &response, uint32_t timeout = 2000); // Gửi lệnh AT và nhận phản hồi
    bool isSimReady(); // Kiểm tra trạng thái SIM (đã sẵn sàng chưa)
    bool isNetworkRegistered(); // Kiểm tra đã đăng ký mạng di động chưa
    String getIMEI(); // Lấy số IMEI của module
    bool sendSMS(const String &phone, const String &message); // Gửi tin nhắn SMS
    bool call(const String &phone); // Thực hiện cuộc gọi
    bool hangUp(); // Kết thúc cuộc gọi
    bool answerCall(); // Trả lời cuộc gọi đến
    int getSignalQuality(); // Lấy chất lượng tín hiệu mạng (RSSI)
    String getModuleInfo(); // Lấy thông tin module (phiên bản, tên, ...)
    // MQTT
    bool mqttConfig(const String &clientId, const String &username = "", const String &password = ""); // Cấu hình MQTT client
    bool mqttSetServer(const String &address, int port, int version = 4); // Thiết lập server MQTT
    bool mqttConnect(int cleanSession = 1, int keepalive = 60); // Kết nối MQTT
    bool mqttPublish(const String &topic, const String &payload, int qos = 0, int retain = 0); // Gửi dữ liệu lên topic MQTT
    bool mqttSubscribe(const String &topic, int qos = 0); // Đăng ký nhận dữ liệu từ topic MQTT
    bool mqttUnsubscribe(const String &topic); // Hủy đăng ký topic MQTT
    bool mqttDisconnect(); // Ngắt kết nối MQTT
    bool mqttClose(); // Đóng kết nối MQTT
    int  mqttStatus(); // Lấy trạng thái MQTT
    bool attachGPRS(const String &apn, const String &user = "", const String &pass = ""); // Gắn mạng GPRS và thiết lập APN
    bool activatePDP(int cid = 1); // Kích hoạt PDP context (bắt đầu truyền dữ liệu)
    bool deactivatePDP(int cid = 1); // Ngắt PDP context (ngắt truyền dữ liệu)
    bool connectInternet4G(const String &apn, const String &user = "", const String &pass = "", int cid = 1); // Kết nối Internet 4G (gộp các bước)
    // Tự động reconnect khi mất mạng
    bool ensureNetwork(uint8_t retry = 3, uint32_t interval = 5000); // Tự động kiểm tra và kết nối lại mạng nếu mất
    String readAT(uint32_t timeout = 2000); // Đọc phản hồi từ module SIM
private:
    HardwareSerial &serial_; // Đối tượng Serial dùng để giao tiếp với module
    uint32_t baud_; // Tốc độ baudrate
};
