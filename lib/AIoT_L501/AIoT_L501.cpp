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
    Serial.println("    ___    ____      _ _ _ _");
    Serial.println("   /   |  /_ _/ ____/_ _ _ /");
    Serial.println("  / /| |  / / / __ \\/ /    ");
    Serial.println(" / ___ |_/ / / /_/  / /     ");
    Serial.println("/_/  |_/___/ \\____/_/      ");
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║           AIoT L501 SDK - ESP32 + 4G Module           ║");
    Serial.println("║                   Version: 1.0                        ║");
    Serial.println("║                   Author: AIoT                        ║");
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
// ADVANCED SMS FEATURES
// ============================================================================

bool AIoT_L501::deleteSMS(int index, int flag) {
    String resp;
    String cmd = "AT+CMGD=" + String(index) + "," + String(flag);
    return sendAT(cmd.c_str(), resp, 2000);
}

int AIoT_L501::saveSMS(const String &phone, const String &message) {
    String resp;
    if (!sendAT("AT+CMGF=1", resp, 2000)) return -1;
    
    serial_.print("AT+CMGW=\"");
    serial_.print(phone);
    serial_.println("\"");
    delay(100);
    serial_.print(message);
    serial_.write(26); // Ctrl+Z
    
    // Response format: +CMGW: <index>
    uint32_t start = millis();
    resp = "";
    while (millis() - start < 10000) {
        while (serial_.available()) {
            char c = serial_.read();
            resp += c;
            if (resp.indexOf("OK") != -1 && resp.indexOf("+CMGW:") != -1) {
                int idx = resp.indexOf("+CMGW:");
                int end = resp.indexOf("\r\n", idx);
                String idxStr = resp.substring(idx + 7, end);
                idxStr.trim();
                return idxStr.toInt();
            }
            if (resp.indexOf("ERROR") != -1) return -1;
        }
    }
    return -1;
}

bool AIoT_L501::sendStoredSMS(int index, const String &phone) {
    String resp;
    String cmd = "AT+CMSS=" + String(index);
    if (phone.length() > 0) {
        cmd += ",\"" + phone + "\"";
    }
    return sendAT(cmd.c_str(), resp, 10000);
}

bool AIoT_L501::setServiceCenter(const String &sc_number) {
    String resp;
    String cmd = "AT+CSCA=\"" + sc_number + "\"";
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::setTextModeParams(int fo, int vp, int pid, int dcs) {
    String resp;
    String cmd = "AT+CSMP=" + String(fo) + "," + String(vp) + "," + String(pid) + "," + String(dcs);
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::setNewMessageIndication(int mode, int mt, int bm, int ds, int bfr) {
    String resp;
    // AT+CNMI=mode,mt,bm,ds,bfr
    String cmd = "AT+CNMI=" + String(mode) + "," + String(mt) + "," + String(bm) + "," + String(ds) + "," + String(bfr);
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::selectMessageService(int service) {
    String resp;
    String cmd = "AT+CSMS=" + String(service);
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::selectStorage(const String &mem1, const String &mem2, const String &mem3) {
    String resp;
    String cmd = "AT+CPMS=\"" + mem1 + "\",\"" + mem2 + "\",\"" + mem3 + "\"";
    return sendAT(cmd.c_str(), resp, 2000);
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
// ADVANCED CALL FEATURES
// ============================================================================

bool AIoT_L501::callEmergency() {
    String resp;
    // Lệnh này có thể thay đổi tùy mạng/module, nhưng theo ảnh user gửi là AT*DIALE
    return sendAT("AT*DIALE", resp, 5000);
}

String AIoT_L501::listCalls() {
    String resp;
    if (sendAT("AT+CLCC", resp, 2000)) {
        return resp; // Trả về raw list, người dùng tự parse
    }
    return "";
}

bool AIoT_L501::forwardCall(int reason, int mode, const String &number, int classx) {
    String resp;
    // AT+CCFC=reason,mode,"number",type,class
    String cmd = "AT+CCFC=" + String(reason) + "," + String(mode);
    if (number.length() > 0) {
        cmd += ",\"" + number + "\",129"; // 129: unknown type/national
    }
    // cmd += "," + String(classx); // Tùy chọn
    return sendAT(cmd.c_str(), resp, 5000);
}

bool AIoT_L501::callWaiting(int mode) {
    String resp;
    // AT+CCWA=[n],[mode],[class]
    // mode=0: disable, 1: enable
    String cmd = "AT+CCWA=0," + String(mode);
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::callHold(int n) {
    String resp;
    // AT+CHLD=n (0: Release held, 1: Release active/accept held, 2: Hold active/accept held...)
    String cmd = "AT+CHLD=" + String(n);
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::restrictSIM(int command, int fileid, int p1, int p2, int p3, const String &data) {
    String resp;
    // AT+CRSM=command,fileid,P1,P2,P3,data
    String cmd = "AT+CRSM=" + String(command) + "," + String(fileid) + "," + 
                 String(p1) + "," + String(p2) + "," + String(p3);
    if (data.length() > 0) {
        cmd += ",\"" + data + "\"";
    }
    return sendAT(cmd.c_str(), resp, 5000);
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

//HTTP-HTTPS
bool AIoT_L501::httpBegin() {
    String resp;
    return sendAT("AT$HTTPOPEN", resp, 2000) && resp.indexOf("OK") != -1;
}
void AIoT_L501::httpStop() {
    String resp;
    sendAT("AT$HTTPCLOSE", resp, 2000);
}
void AIoT_L501::parseUrl(const String &url, String &host, int &port, int &isHttps) { //Phân tích URL để tách Port và xác định HTTPS
    isHttps = 0;
    port = 80;
    if (url.startsWith("https://")) {
        isHttps = 1;
        port = 443;
    }
    host = url; 
}
String AIoT_L501::readHttpBody(uint32_t timeout) { //Đọc kết quả phản hồi từ Server
    String data = "";
    uint32_t start = millis();
    bool capture = false;
    while (millis() - start < timeout) {
        if (serial_.available()) {
            String line = serial_.readStringUntil('\n');
            line.trim();
            if (line.startsWith("$HTTPRECV:DATA")) {
                capture = true;
                continue; 
            }
            if (line.startsWith("$HTTPERROR")) {
                return "ERROR: " + line;
            }
            if (capture) {
                data += line + "\n";
            }
        }
    }
    if (data.length() == 0) return "TIMEOUT_OR_NO_DATA";
    return data;
}
String AIoT_L501::httpGET(const String &url) { //Thực hiện GET
    String host, resp;
    int port, isHttps;
    parseUrl(url, host, port, isHttps);
    String cmdPara = "AT$HTTPPARA=\"" + host + "\"," + String(port) + "," + String(isHttps);
    if (!sendAT(cmdPara.c_str(), resp, 2000) || resp.indexOf("OK") == -1) {
        return "ERROR_CONFIG";
    }
    serial_.println("AT$HTTPACTION=0");
    return readHttpBody(15000);
}
String AIoT_L501::httpPOST(const String &url, const String &contentType, const String &data) { //Thực hiện POST
    String host, resp;
    int port, isHttps;
    parseUrl(url, host, port, isHttps);
    String cmdPara = "AT$HTTPPARA=\"" + host + "\"," + String(port) + "," + String(isHttps);
    if (!sendAT(cmdPara.c_str(), resp, 2000) || resp.indexOf("OK") == -1) {
        return "ERROR_CONFIG";
    }
    String cmdType = "AT$HTTPRQH=\"Content-Type\",\"" + contentType + "\"";
    sendAT(cmdType.c_str(), resp, 2000);
    String cmdLen = "AT$HTTPRQH=\"Content-Length\",\"" + String(data.length()) + "\"";
    sendAT(cmdLen.c_str(), resp, 2000);
    String cmdData = "AT$HTTPDATAEX=" + String(data.length()) + ",\"" + data + "\"";
    if (!sendAT(cmdData.c_str(), resp, 5000) || resp.indexOf("OK") == -1) {
        return "ERROR_DATA_SET";
    }
    serial_.println("AT$HTTPACTION=3");
    return readHttpBody(15000);
}

// ============================================================================
// ADVANCED HTTP FEATURES (L501 Specific)
// ============================================================================

bool AIoT_L501::httpSetHeader(const String &name, const String &value) {
    String resp;
    String cmd = "AT$HTTPRQH=\"" + name + "\",\"" + value + "\"";
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::httpSetSaveLocation(int type) {
    String resp;
    // AT$HTTPTYPE=0 (RAM), 1 (File)
    String cmd = "AT$HTTPTYPE=" + String(type);
    return sendAT(cmd.c_str(), resp, 2000);
}

String AIoT_L501::httpReadFromFile(uint32_t len, uint32_t timeout) {
    // AT$HTTPREAD=<len>
    String resp;
    String cmd = "AT$HTTPREAD=" + String(len);
    serial_.println(cmd);
    return readHttpBody(timeout); 
}

String AIoT_L501::httpPostStandard(const String &url, const String &contentType, const String &data) {
    String host, resp;
    int port, isHttps;
    parseUrl(url, host, port, isHttps);
    
    // 1. Config Param
    String cmdPara = "AT$HTTPPARA=\"" + host + "\"," + String(port) + "," + String(isHttps);
    if (!sendAT(cmdPara.c_str(), resp, 2000) || resp.indexOf("OK") == -1) {
        return "ERROR_CONFIG";
    }
    
    // 2. Head
    httpSetHeader("Content-Type", contentType);
    httpSetHeader("Content-Length", String(data.length()));
    
    // 3. Set Data Length
    String cmdData = "AT$HTTPDATA=" + String(data.length());
    // Giả định AT$HTTPDATA trả về CONNECT hoặc > để nhập liệu
    serial_.println(cmdData);
    delay(100);
    serial_.print(data);
    
    // Chờ OK xác nhận data đã buffer xong
    resp = readAT(5000); 
    
    // 4. Trigger Send (AT$HTTPSEND)
    serial_.println("AT$HTTPSEND"); 
    
    return readHttpBody(15000);
}

// ============================================================================
// USSD
// ============================================================================

bool AIoT_L501::sendUSSD(const String &ussd) {
    String resp;
    String cmd = "AT+CUSD=1,\"" + ussd + "\"";
    return sendAT(cmd.c_str(), resp, 10000); // Timeout 10s cho USSD
}

bool AIoT_L501::cancelUSSD() {
    String resp;
    return sendAT("AT+CUSD=2", resp, 5000);
}

// ============================================================================
// LOW POWER
// ============================================================================

bool AIoT_L501::sleep(bool enable) {
    String resp;
    // AT+CSCLK=1: Enable auto sleep, =0: Disable
    String cmd = enable ? "AT+CSCLK=1" : "AT+CSCLK=0";
    return sendAT(cmd.c_str(), resp, 2000);
}

// ============================================================================
// AUDIO / DTMF (Interface Only - Chưa implement)
// ============================================================================

bool AIoT_L501::setVolume(int level) {
    // TODO: Implement AT+CLVL=level or equivalent
    return false;
}

int AIoT_L501::getVolume() {
    // TODO: Implement query
    return -1;
}

bool AIoT_L501::sendDTMF(const String &dtmf) {
    // TODO: Implement AT+VTS=...
    return false;
}

// ============================================================================
// SOCKET TCP/UDP (Standard AT Commands)
// ============================================================================

bool AIoT_L501::socketBegin() {
    String resp;
    // Cấu hình PDP context nếu chưa connect
    // Thường đã được làm trong ensureNetwork -> activatePDP
    return true;
}

bool AIoT_L501::socketConnect(const char* type, const char* host, int port) {
    String resp;
    // AT+CIPSTART="TCP","IP","Port"
    String cmd = "AT+CIPSTART=\"" + String(type) + "\",\"" + String(host) + "\"," + String(port);
    if (!sendAT(cmd.c_str(), resp, 15000)) return false;
    
    // Kiểm tra phản hồi, thường là "CONNECT OK" hoặc "ALREADY CONNECT"
    if (resp.indexOf("CONNECT OK") != -1 || resp.indexOf("ALREADY CONNECT") != -1) {
        return true;
    }
    return false;
}

bool AIoT_L501::socketSend(const String &data) {
    String resp;
    String cmd = "AT+CIPSEND=" + String(data.length());
    // Gửi lệnh báo độ dài
    serial_.println(cmd);
    delay(200); // Ổn định
    
    // Đợi dấu nhắc '>', nếu thư viện không check kịp thì gửi luôn
    // Ở đây gửi luôn data
    serial_.print(data);
    
    // Chờ phản hồi SEND OK
    resp = readAT(5000);
    return resp.indexOf("SEND OK") != -1;
}

String AIoT_L501::socketRead(uint32_t timeout) {
    // Đọc dữ liệu socket đến
    // Module thường trả về: +IPD,len:data
    return readAT(timeout);
}

bool AIoT_L501::socketClose() {
    String resp;
    return sendAT("AT+CIPCLOSE", resp, 5000);
}

bool AIoT_L501::socketStatus() {
    String resp;
    if (sendAT("AT+CIPSTATUS", resp, 3000)) {
        return resp.indexOf("STATE: CONNECT OK") != -1;
    }
    return false;
}

// ============================================================================
// ADVANCED TCP/IP FEATURES (L501 Specific)
// ============================================================================

bool AIoT_L501::netConfigAPN(int contextID, int contextType, const String &apn, const String &user, const String &pass, int auth) {
    String resp;
    // AT+QICSGP=contextID,contextType,"apn","username","password",auth
    String cmd = "AT+QICSGP=" + String(contextID) + "," + String(contextType) + ",\"" + apn + "\"";
    if (user.length() > 0) {
        cmd += ",\"" + user + "\",\"" + pass + "\"," + String(auth);
    }
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::netOpen() {
    String resp;
    return sendAT("AT+NETOPEN", resp, 5000) || resp.indexOf("opened") != -1;
}

bool AIoT_L501::netClose() {
    String resp;
    return sendAT("AT+NETCLOSE", resp, 5000);
}

bool AIoT_L501::netSetDNS(const String &dns1, const String &dns2) {
    String resp;
    String cmd = "AT+NETDNS=\"" + dns1 + "\"";
    if (dns2.length() > 0) {
        cmd += ",\"" + dns2 + "\"";
    }
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::socketOpen(int connectID, const String &type, const String &host, int port) {
    String resp;
    // AT+CIPOPEN=connectID,"type","host",port
    String cmd = "AT+CIPOPEN=" + String(connectID) + ",\"" + type + "\",\"" + host + "\"," + String(port);
    // Lệnh này có thể mất nhiều thời gian để connect
    if (!sendAT(cmd.c_str(), resp, 15000)) return false;
    return resp.indexOf("OK") != -1 && resp.indexOf("OPEN") != -1;
}

bool AIoT_L501::socketManualGet(int mode) {
    String resp;
    String cmd = "AT+CIPRXGET=" + String(mode);
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::socketTransMode(int mode) {
    String resp;
    String cmd = "AT+CIPMODE=" + String(mode);
    return sendAT(cmd.c_str(), resp, 2000);
}

String AIoT_L501::ping(const String &host) {
    String resp;
    String cmd = "AT+MPING=\"" + host + "\"";
    if (sendAT(cmd.c_str(), resp, 10000)) {
        return resp; // Trả về kết quả ping raw
    }
    return "";
}

bool AIoT_L501::serverStart(int port) {
    String resp;
    String cmd = "AT+SERVERSTART=" + String(port);
    return sendAT(cmd.c_str(), resp, 5000);
}

bool AIoT_L501::serverStop() {
    String resp;
    return sendAT("AT+SERVERSTOP", resp, 2000);
}

String AIoT_L501::dnsLookup(const String &domain) {
    String resp;
    String cmd = "AT+MDNSGIP=\"" + domain + "\"";
    if (sendAT(cmd.c_str(), resp, 10000)) {
        return resp;
    }
    return "";
}

String AIoT_L501::dataUsage() {
    String resp;
    if (sendAT("AT+USEDDATA", resp, 2000)) {
        return resp;
    }
    return "";
}

// ============================================================================
// FILE SYSTEM (L501 Specific AT+MFS...)
// ============================================================================

bool AIoT_L501::fileSave(const String &filename, const String &content) {
    // Wrapper: Create -> Write
    if (!fsCreate(filename)) return false;
    return fsWrite(filename, 0, content);
}

String AIoT_L501::fileLoad(const String &filename) {
    String resp;
    // AT+MFSREAD=<filename>
    String cmd = "AT+MFSREAD=\"" + filename + "\"";
    if (sendAT(cmd.c_str(), resp, 5000)) {
        return resp; 
    }
    return "";
}

bool AIoT_L501::fileDel(const String &filename) {
    String resp;
    String cmd = "AT+MFSDEL=\"" + filename + "\"";
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::fsCD(const String &path) {
    String resp;
    String cmd = "AT+MFSCD=\"" + path + "\"";
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::fsMkdir(const String &path) {
    String resp;
    String cmd = "AT+MFSMKDIR=\"" + path + "\"";
    return sendAT(cmd.c_str(), resp, 2000);
}

String AIoT_L501::fsLs() {
    String resp;
    if (sendAT("AT+MFSLS", resp, 5000)) {
        return resp;
    }
    return "";
}

bool AIoT_L501::fsCreate(const String &filename) {
    String resp;
    // AT+MFSCREATE="filename"
    String cmd = "AT+MFSCREATE=\"" + filename + "\"";
    return sendAT(cmd.c_str(), resp, 2000);
}

String AIoT_L501::fsMem() {
    String resp;
    if (sendAT("AT+MFSMEM", resp, 2000)) {
        return resp;
    }
    return "";
}

bool AIoT_L501::fsCopy(const String &src, const String &dest) {
    String resp;
    String cmd = "AT+MFSCOPY=\"" + src + "\",\"" + dest + "\"";
    return sendAT(cmd.c_str(), resp, 5000);
}

bool AIoT_L501::fsWrite(const String &filename, uint32_t offset, const String &data) {
    String resp;
    // AT+MFSWRITE="filename",offset,length,data
    // Dữ liệu string nên cẩn thận ký tự đặc biệt
    String cmd = "AT+MFSWRITE=\"" + filename + "\"," + String(offset) + "," + String(data.length()) + ",\"" + data + "\"";
    return sendAT(cmd.c_str(), resp, 5000);
}

// ============================================================================
// UTILITIES (NTP, CLOCK...)
// ============================================================================

String AIoT_L501::getClock() {
    String resp;
    if (sendAT("AT+CCLK?", resp, 2000)) {
        int idx = resp.indexOf("+CCLK:");
        if (idx != -1) {
            int end = resp.indexOf("\r\n", idx);
            if (end != -1) return resp.substring(idx + 7, end);
        }
    }
    return "";
}

bool AIoT_L501::syncNTP(const String &server, int timezone) {
    String resp;
    // AT+NTP="server",timezone
    String cmd = "AT+NTP=\"" + server + "\"," + String(timezone);
    return sendAT(cmd.c_str(), resp, 10000);
}

// ============================================================================
// SSL/TLS (Standard AT)
// ============================================================================

bool AIoT_L501::sslConfig(int ssl_ctx_index, int verify_mode, int security_level) {
    String resp;
    // AT+CSSLCFG="sslversion",ctx,3 (TLS 1.2)
    // AT+CSSLCFG="authmode",ctx,verify_mode
    String cmd = "AT+CSSLCFG=\"sslversion\"," + String(ssl_ctx_index) + ",3"; 
    sendAT(cmd.c_str(), resp, 2000);
    
    cmd = "AT+CSSLCFG=\"authmode\"," + String(ssl_ctx_index) + "," + String(verify_mode);
    return sendAT(cmd.c_str(), resp, 2000);
}

// ============================================================================
// SYSTEM CONTROL (Advanced)
// ============================================================================

bool AIoT_L501::switchToCommandMode() {
    // Sequence: Wait 1s -> Send "+++" -> Wait 1s
    delay(1000);
    serial_.print("+++");
    delay(1000);
    // Check response (thường là OK hoặc NO CARRIER nếu mất kết nối)
    String resp = readAT(2000);
    return resp.indexOf("OK") != -1;
}

bool AIoT_L501::switchToDataMode() {
    String resp;
    return sendAT("ATO", resp, 2000);
}

bool AIoT_L501::setAutoAnswer(int rings) {
    String resp;
    String cmd = "ATS0=" + String(rings);
    return sendAT(cmd.c_str(), resp, 2000);
}

bool AIoT_L501::factoryReset() {
    String resp;
    return sendAT("AT&F", resp, 5000);
}