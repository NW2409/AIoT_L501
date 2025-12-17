#include "AIoT_L501.h"

// ============================================================================
// CONSTRUCTOR & KHỞI TẠO
// ============================================================================

AIoT_L501::AIoT_L501(HardwareSerial &serial, uint32_t baud)
    : serial_(serial), baud_(baud) {}

void AIoT_L501::begin() {
    serial_.begin(baud_);
}

bool AIoT_L501::init(unsigned long timeout) {
    // In logo AIoT
    Serial.println();
    Serial.println("    ___    ____      _ _ _ _ ");
    Serial.println("   /   |  /_ _/ ____/_ _ _ /");
    Serial.println("  / /| |  / / / __ \\/ /    ");
    Serial.println(" / ___ |_/ / / /_/  / /     ");
    Serial.println("/_/  |_/___/ \\____/_/      ");
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║           AIoT L501 SDK - ESP32 + 4G Module           ║");
    Serial.println("║                   Version: 1.0                        ║");
    Serial.println("║              Author: AIoT                             ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    Serial.println();

    // Khởi tạo Serial cho module
    serial_.begin(baud_);

    String resp;

    // ========== Kiểm tra AT ==========
    Serial.print("[AIoT] AT -> ");
    unsigned long start = millis();
    bool moduleReady = false;
    while (millis() - start < timeout) {
        if (sendAT("AT", resp, 1000)) {
            moduleReady = true;
            break;
        }
        delay(500);
    }
    if (moduleReady) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
        return false;
    }

    // Tắt echo
    sendAT("ATE0", resp, 1000);

    // ========== Kiểm tra AT+CSQ (Chất lượng tín hiệu) ==========
    Serial.print("[AIoT] AT+CSQ -> ");
    if (sendAT("AT+CSQ", resp, 2000)) {
        int idx = resp.indexOf("+CSQ:");
        if (idx != -1) {
            int end = resp.indexOf("\r\n", idx);
            if (end != -1) {
                String csq = resp.substring(idx, end);
                Serial.println(csq);
            } else {
                Serial.println("OK");
            }
        } else {
            Serial.println("OK");
        }
    } else {
        Serial.println("FAILED");
    }

    // ========== Kiểm tra AT+CPIN? (Khe cắm SIM) ==========
    Serial.print("[AIoT] AT+CPIN? -> ");
    if (sendAT("AT+CPIN?", resp, 2000)) {
        int idx = resp.indexOf("+CPIN:");
        if (idx != -1) {
            int end = resp.indexOf("\r\n", idx);
            if (end != -1) {
                String cpin = resp.substring(idx, end);
                Serial.println(cpin);
            } else {
                Serial.println("OK");
            }
        } else {
            Serial.println("OK");
        }
    } else {
        Serial.println("FAILED (Không có SIM hoặc SIM lỗi)");
        return false;
    }

    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║            ✓ KHỞI TẠO MODULE THÀNH CÔNG!              ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    Serial.println();

    return true;
}

// ============================================================================
// LỆNH AT CƠ BẢN
// ============================================================================

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

String AIoT_L501::readAT(uint32_t timeout) {
    String response = "";
    uint32_t start = millis();
    while (millis() - start < timeout) {
        while (serial_.available()) {
            char c = serial_.read();
            response += c;
        }
        if (response.length() > 0 && (response.endsWith("OK\r\n") || response.endsWith("ERROR\r\n"))) {
            break;
        }
    }
    return response;
}

// ============================================================================
// TRẠNG THÁI MODULE & SIM
// ============================================================================

bool AIoT_L501::isSimReady() {
    String resp;
    if (sendAT("AT+CPIN?", resp, 2000)) {
        return resp.indexOf("READY") != -1;
    }
    return false;
}

bool AIoT_L501::isNetworkRegistered() {
    String resp;
    if (sendAT("AT+CREG?", resp, 2000)) {
        return resp.indexOf(",1") != -1 || resp.indexOf(",5") != -1;
    }
    return false;
}

bool AIoT_L501::isDataConnected() {
    String resp;
    if (sendAT("AT+CGACT?", resp, 5000)) {
        return resp.indexOf(",1") != -1;
    }
    return false;
}

String AIoT_L501::getIMEI() {
    String resp;
    if (sendAT("AT+GSN", resp, 2000)) {
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

int AIoT_L501::getSignalQuality() {
    String resp;
    if (sendAT("AT+CSQ", resp, 2000)) {
        int idx = resp.indexOf("+CSQ:");
        if (idx != -1) {
            int comma = resp.indexOf(",", idx);
            if (comma != -1) {
                String rssi = resp.substring(idx + 6, comma);
                return rssi.toInt();
            }
        }
    }
    return -1;
}

String AIoT_L501::getModuleInfo() {
    String resp;
    if (sendAT("ATI", resp, 2000)) {
        return resp;
    }
    return "";
}

String AIoT_L501::getIPAddress() {
    String resp;
    if (sendAT("AT+CGPADDR=1", resp, 5000)) {
        int idx = resp.indexOf(",\"");
        if (idx != -1) {
            int idx2 = resp.indexOf("\"", idx + 2);
            if (idx2 != -1) {
                return resp.substring(idx + 2, idx2);
            }
        }
    }
    return "";
}

// ============================================================================
// KẾT NỐI MẠNG 4G
// ============================================================================

bool AIoT_L501::ensureNetwork(uint8_t retry, uint32_t interval) {
    for (uint8_t i = 0; i < retry; ++i) {
        if (isSimReady() && isNetworkRegistered()) {
            return true;
        }
        String resp;
        sendAT("AT+CFUN=1", resp, 3000);
        sendAT("AT+CGATT=1", resp, 5000);
        delay(interval);
    }
    return false;
}

bool AIoT_L501::attachGPRS(const String &apn, const String &user, const String &pass) {
    String resp;
    if (!sendAT("AT+CGATT=1", resp, 5000)) return false;
    String cmd = "AT+CGDCONT=1,\"IP\",\"" + apn + "\"";
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
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

bool AIoT_L501::connectInternet4G(const String &apn, const String &user, const String &pass, int cid) {
    String resp;
    if (!sendAT("AT+CGATT=1", resp, 5000)) return false;
    String cmd = "AT+CGDCONT=" + String(cid) + ",\"IP\",\"" + apn + "\"";
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    cmd = "AT+CGACT=1," + String(cid);
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    return true;
}

// ============================================================================
// SMS
// ============================================================================

bool AIoT_L501::sendSMS(const String &phone, const String &message) {
    String resp;
    if (!sendAT("AT+CMGF=1", resp, 2000)) return false;
    serial_.print("AT+CMGS=\"");
    serial_.print(phone);
    serial_.println("\"");
    delay(100);
    serial_.print(message);
    serial_.write(26);
    uint32_t start = millis();
    resp = "";
    while (millis() - start < 10000) {
        while (serial_.available()) {
            char c = serial_.read();
            resp += c;
            if (resp.indexOf("OK") != -1) return true;
            if (resp.indexOf("ERROR") != -1) return false;
        }
    }
    return false;
}

String AIoT_L501::readSMS(int index) {
    String resp;
    sendAT("AT+CMGF=1", resp, 2000);
    sendAT("AT+CPMS=\"SM\",\"SM\",\"SM\"", resp, 3000);
    String cmd = "AT+CMGR=" + String(index);
    if (sendAT(cmd.c_str(), resp, 5000)) {
        if (resp.indexOf("ERROR") != -1) {
            return "";
        }
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

String AIoT_L501::listAllSMS() {
    String resp;
    sendAT("AT+CMGF=1", resp, 2000);
    sendAT("AT+CPMS=\"SM\",\"SM\",\"SM\"", resp, 3000);
    if (sendAT("AT+CMGL=\"ALL\"", resp, 10000)) {
        return resp;
    }
    return "";
}

// ============================================================================
// CUỘC GỌI
// ============================================================================

bool AIoT_L501::call(const String &phone) {
    String resp;
    String cmd = "ATD" + phone + ";";
    return sendAT(cmd.c_str(), resp, 5000);
}

bool AIoT_L501::hangUp() {
    String resp;
    return sendAT("ATH", resp, 2000);
}

bool AIoT_L501::answerCall() {
    String resp;
    return sendAT("ATA", resp, 2000);
}

// ============================================================================
// MQTT
// ============================================================================

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
    if (!sendAT(cmd.c_str(), resp, 15000)) return false;
    String resp2 = readAT(10000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

bool AIoT_L501::mqttConnect(int cleanSession, int keepalive) {
    String resp;
    String cmd = "AT+MCONNECT=" + String(cleanSession) + "," + String(keepalive);
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    String resp2 = readAT(10000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

bool AIoT_L501::mqttPublish(const String &topic, const String &payload, int qos, int retain) {
    String resp;
    String cmd = "AT+MPUB=\"" + topic + "\"," + String(qos) + "," + String(retain) + ",\"" + payload + "\"";
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    String resp2 = readAT(5000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

bool AIoT_L501::mqttSubscribe(const String &topic, int qos) {
    String resp;
    String cmd = "AT+MSUB=\"" + topic + "\"," + String(qos);
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    String resp2 = readAT(5000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

bool AIoT_L501::mqttUnsubscribe(const String &topic) {
    String resp;
    String cmd = "AT+MUNSUB=\"" + topic + "\"";
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    String resp2 = readAT(5000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

bool AIoT_L501::mqttDisconnect() {
    String resp;
    if (!sendAT("AT+MDISCONNECT", resp, 5000)) return false;
    String resp2 = readAT(5000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

bool AIoT_L501::mqttClose() {
    String resp;
    if (!sendAT("AT+MIPCLOSE", resp, 5000)) return false;
    String resp2 = readAT(5000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

int AIoT_L501::mqttStatus() {
    String resp;
    if (sendAT("AT+MQTTSTATU", resp, 2000)) {
        int idx = resp.indexOf("+MQTTSTATU:");
        if (idx != -1) {
            int end = resp.indexOf("\r\n", idx);
            if (end != -1) {
                String status = resp.substring(idx + 11, end);
                status.trim();
                return status.toInt();
            }
        }
    }
    return 0;
}

String AIoT_L501::mqttReceive(uint32_t timeout) {
    String data = readAT(timeout);
    if (data.indexOf("+MSUB:") != -1) {
        return data;
    }
    return "";
}