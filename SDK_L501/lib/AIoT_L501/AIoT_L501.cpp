#include "AIoT_L501.h"

AIoT_L501::AIoT_L501(HardwareSerial &serial, uint32_t baud)
    : serial_(serial), baud_(baud) {}

void AIoT_L501::begin() {
    serial_.begin(baud_);
}

bool AIoT_L501::sendAT(const char *cmd, String &response, uint32_t timeout) {
    serial_.println(cmd);
    uint32_t start = millis();
    response = "";
    while (millis() - start < timeout) {
        while (serial_.available()) {
            char c = serial_.read();
            response += c;
            if (response.endsWith("OK\r\n") || response.endsWith("ERROR\r\n")) {
                return response.indexOf("OK") != -1;
            }
        }
    }
    return false;
}

bool AIoT_L501::isSimReady() {
    String resp;
    if (sendAT("AT+CPIN?", resp)) {
        return resp.indexOf("READY") != -1;
    }
    return false;
}

bool AIoT_L501::isNetworkRegistered() {
    String resp;
    if (sendAT("AT+CREG?", resp)) {
        // Tìm "+CREG: x,1" hoặc "+CREG: x,5" (đã đăng ký mạng)
        return resp.indexOf(",1") != -1 || resp.indexOf(",5") != -1;
    }
    return false;
}

String AIoT_L501::getIMEI() {
    String resp;
    if (sendAT("AT+GSN", resp)) {
        // IMEI thường nằm ở dòng thứ 2
        int idx = resp.indexOf("\r\n");
        if (idx != -1) {
            int idx2 = resp.indexOf("\r\n", idx + 2);
            if (idx2 != -1) {
                return resp.substring(idx + 2, idx2);
            }
        }
    }
    return "";
}

bool AIoT_L501::sendSMS(const String &phone, const String &message) {
    String resp;
    if (!sendAT("AT+CMGF=1", resp)) return false; // Chuyển sang chế độ text
    serial_.print("AT+CMGS=\"");
    serial_.print(phone);
    serial_.println("\"");
    delay(100);
    serial_.print(message);
    serial_.write(26); // Ctrl+Z kết thúc tin nhắn
    uint32_t start = millis();
    resp = "";
    while (millis() - start < 10000) { // Đợi tối đa 10 giây
        while (serial_.available()) {
            char c = serial_.read();
            resp += c;
            if (resp.indexOf("OK") != -1) return true;
            if (resp.indexOf("ERROR") != -1) return false;
        }
    }
    return false;
}

bool AIoT_L501::call(const String &phone) {
    String resp;
    String cmd = "ATD" + phone + ";";
    if (sendAT(cmd.c_str(), resp)) {
        // Nếu nhận OK thì bắt đầu gọi
        return resp.indexOf("OK") != -1;
    }
    return false;
}

bool AIoT_L501::hangUp() {
    String resp;
    return sendAT("ATH", resp);
}

bool AIoT_L501::answerCall() {
    String resp;
    return sendAT("ATA", resp);
}

int AIoT_L501::getSignalQuality() {
    String resp;
    if (sendAT("AT+CSQ", resp)) {
        int idx = resp.indexOf("+CSQ:");
        if (idx != -1) {
            int comma = resp.indexOf(",", idx);
            if (comma != -1) {
                String rssi = resp.substring(idx + 6, comma);
                return rssi.toInt(); // Giá trị RSSI (0-31, 99: không xác định)
            }
        }
    }
    return -1; // Lỗi hoặc không xác định
}

String AIoT_L501::getModuleInfo() {
    String resp;
    if (sendAT("ATI", resp)) {
        return resp;
    }
    return "";
}

bool AIoT_L501::mqttConfig(const String &clientId, const String &username, const String &password) {
    String resp;
    String cmd = "AT+MCONFIG=\"" + clientId + "\"";
    if (username.length() > 0) {
        cmd += ",\"" + username + "\"";
        if (password.length() > 0) {
            cmd += ",\"" + password + "\"";
        }
    }
    return sendAT(cmd.c_str(), resp, 5000);
}

bool AIoT_L501::mqttSetServer(const String &address, int port, int version) {
    String resp;
    String cmd = "AT+MIPSTART=\"" + address + "\"," + String(port) + "," + String(version);
    return sendAT(cmd.c_str(), resp, 10000);
}

bool AIoT_L501::mqttConnect(int cleanSession, int keepalive) {
    String resp;
    String cmd = "AT+MCONNECT=" + String(cleanSession) + "," + String(keepalive);
    return sendAT(cmd.c_str(), resp, 10000) && resp.indexOf("SUCCESS") != -1;
}

bool AIoT_L501::mqttPublish(const String &topic, const String &payload, int qos, int retain) {
    String resp;
    String cmd = "AT+MPUB=\"" + topic + "\"," + String(qos) + "," + String(retain) + ",\"" + payload + "\"";
    return sendAT(cmd.c_str(), resp, 10000) && resp.indexOf("SUCCESS") != -1;
}

bool AIoT_L501::mqttSubscribe(const String &topic, int qos) {
    String resp;
    String cmd = "AT+MSUB=\"" + topic + "\"," + String(qos);
    return sendAT(cmd.c_str(), resp, 10000) && resp.indexOf("SUCCESS") != -1;
}

bool AIoT_L501::mqttUnsubscribe(const String &topic) {
    String resp;
    String cmd = "AT+MUNSUB=\"" + topic + "\"";
    return sendAT(cmd.c_str(), resp, 10000) && resp.indexOf("SUCCESS") != -1;
}

bool AIoT_L501::mqttDisconnect() {
    String resp;
    return sendAT("AT+MDISCONNECT", resp, 5000) && resp.indexOf("SUCCESS") != -1;
}

bool AIoT_L501::mqttClose() {
    String resp;
    return sendAT("AT+MIPCLOSE", resp, 5000) && resp.indexOf("SUCCESS") != -1;
}

int AIoT_L501::mqttStatus() {
    String resp;
    if (sendAT("AT+MQTTSTATU", resp, 2000)) {
        int idx = resp.indexOf("+MQTTSTATU:");
        if (idx != -1) {
            int end = resp.indexOf("\r\n", idx);
            String status = resp.substring(idx + 11, end);
            status.trim();
            return status.toInt();
        }
    }
    return 0;
}

bool AIoT_L501::attachGPRS(const String &apn, const String &user, const String &pass) {
    String resp;
    // Gắn mạng
    if (!sendAT("AT+CGATT=1", resp, 5000)) return false;
    // Thiết lập APN
    String cmd = "AT+CGDCONT=1,\"IP\",\"" + apn + "\"";
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    // Nếu module không hỗ trợ AT+CGAUTH thì bỏ qua user/pass
    return true;
}

bool AIoT_L501::activatePDP(int cid) {
    String resp;
    String cmd = "AT+CGACT=1," + String(cid);
    return sendAT(cmd.c_str(), resp, 5000);
}

bool AIoT_L501::deactivatePDP(int cid) {
    String resp;
    String cmd = "AT+CGACT=0," + String(cid);
    return sendAT(cmd.c_str(), resp, 5000);
}