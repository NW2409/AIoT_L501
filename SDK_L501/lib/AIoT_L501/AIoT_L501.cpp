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
    Serial.println("                ___    ____    _________");
    Serial.println("               /   |  /  _/___/___  ___/");
    Serial.println("              / /| |  / / / __ \\/ /    ");
    Serial.println("             / ___ |_/ / / /_/ / /    ");
    Serial.println("            /_/  |_/___/ \\____/_/     ");
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
    if (sendAT("AT+CPIN?", resp, 3000)) {
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

// Xóa buffer Serial
void AIoT_L501::clearBuffer() {
    while (serial_.available()) {
        serial_.read();
    }
}

bool AIoT_L501::sendAT(const char *cmd, String &response, uint32_t timeout) {
    // Xóa buffer trước khi gửi
    //clearBuffer();
    
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

bool AIoT_L501::setBaudrate(uint32_t baud, uint32_t timeout) {
    String resp;
    uint32_t oldBaud = baud_;

    // Bắt tay trước khi đổi
    sendAT("AT", resp, 500);

    // Đổi baudrate trên module SIM
    String cmd = "AT+IPR=" + String(baud);
    if (!sendAT(cmd.c_str(), resp, timeout)) {
        return false;
    }

    // Đổi UART ESP32 sang baud mới
    baud_ = baud;
    serial_.begin(baud_);

    // Bắt tay lại xác nhận liên kết hoạt động ở baud mới
    unsigned long start = millis();
    bool ok = false;
    while (millis() - start < 2000) {
        if (sendAT("AT", resp, 300)) { ok = true; break; }
        delay(100);
    }
    if (!ok) {
        // Rollback UART nếu handshake thất bại
        baud_ = oldBaud;
        serial_.begin(baud_);
        sendAT("AT", resp, 500);
        return false;
    }
    return true;
}

uint32_t AIoT_L501::getBaudrate() const {
    return baud_;
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
    if (!sendAT("AT+CGACT?", resp, 5000)) return false;

    // Parse đúng từng dòng: +CGACT: <cid>,<status>
    int pos = 0;
    while ((pos = resp.indexOf("+CGACT:", pos)) != -1) {
        int eol = resp.indexOf("\r\n", pos);
        String line = (eol != -1) ? resp.substring(pos, eol) : resp.substring(pos);
        line.trim();

        int colon = line.indexOf(':');
        if (colon != -1) {
            String after = line.substring(colon + 1);
            after.trim(); // "cid,status"
            int comma = after.indexOf(',');
            if (comma != -1) {
                String statusStr = after.substring(comma + 1);
                statusStr.trim();
                if (statusStr == "1") return true; // có ít nhất 1 PDP active
            }
        }
        pos = (eol != -1) ? eol : pos + 1;
    }
    return false;
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

    // Thử hỏi theo CID=1 trước
    bool ok = sendAT("AT+CGPADDR=1", resp, 2000);
    if (!ok || resp.indexOf("+CGPADDR:") == -1) {
        // Fallback: hỏi tất cả
        sendAT("AT+CGPADDR", resp, 2000);
    }

    // Tìm địa chỉ nằm trong dấu nháy "..."
    String ipv6 = "";
    int pos = 0;
    while (true) {
        int q1 = resp.indexOf('\"', pos);
        if (q1 == -1) break;
        int q2 = resp.indexOf('\"', q1 + 1);
        if (q2 == -1) break;
        String tok = resp.substring(q1 + 1, q2);
        tok.trim();

        bool isIPv4 = (tok.indexOf('.') != -1) && (tok.indexOf(':') == -1);
        bool isIPv6 = (tok.indexOf(':') != -1);

        if (isIPv4) return tok;   // ưu tiên IPv4
        if (isIPv6 && ipv6.length() == 0) ipv6 = tok;

        pos = q2 + 1;
    }

    // Nếu không có dấu nháy, thử parse sau dấu phẩy đầu tiên
    int idx = resp.indexOf("+CGPADDR:");
    if (idx != -1) {
        int comma = resp.indexOf(',', idx);
        if (comma != -1) {
            int eol = resp.indexOf("\r\n", comma);
            String tail = (eol != -1) ? resp.substring(comma + 1, eol) : resp.substring(comma + 1);
            tail.trim();
            if (tail.length() > 0) return tail;
        }
    }

    // Trả về IPv6 nếu không có IPv4
    return ipv6;
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

// AT+NETOPEN - Mở kết nối mạng
// bool AIoT_L501::netOpen() {
//     String resp;
//     if (!sendAT("AT+NETOPEN", resp, 10000)) return false;
//     String resp2 = readAT(5000);
//     return (resp.indexOf("OK") != -1) || (resp2.indexOf("SUCCESS") != -1);
// }

// // AT+NETCLOSE - Đóng kết nối mạng
// bool AIoT_L501::netClose() {
//     String resp;
//     if (!sendAT("AT+NETCLOSE", resp, 5000)) return false;
//     String resp2 = readAT(5000);
//     return (resp.indexOf("OK") != -1) || (resp2.indexOf("SUCCESS") != -1);
// }

bool AIoT_L501::connectInternet4G(const String &apn, const String &user, const String &pass, int cid) {
    String resp;
    if (!sendAT("AT+CGATT=1", resp, 5000)) return false;
    String cmd = "AT+CGDCONT=" + String(cid) + ",\"IP\",\"" + apn + "\"";
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    cmd = "AT+CGACT=1," + String(cid);
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    
    // Mở kết nối mạng
    if (!netOpen()) return false;
    
    return true;
}

// ============================================================================
// SMS
// ============================================================================

bool AIoT_L501::sendSMS(const String &phone, const String &message) {
    String resp;
    if (!sendAT("AT+CMGF=1", resp, 2000)) return false;
    
    clearBuffer();
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
    String resp, tmp;

    // Bật text mode
    sendAT("AT+CMGF=1", tmp, 2000);

    // Thử lần lượt 2 bộ nhớ: SIM (SM) rồi bộ nhớ máy (ME)
    const char* stores[] = {"SM", "ME"};
    for (int i = 0; i < 2; ++i) {
        String cpms = String("AT+CPMS=\"") + stores[i] + "\",\"" + stores[i] + "\",\"" + stores[i] + "\"";
        sendAT(cpms.c_str(), tmp, 3000);

        // Gửi CMGL="ALL" và đọc dài hơn vì dữ liệu nhiều
        clearBuffer();
        HardwareSerial &s = serial_;
        s.println("AT+CMGL=\"ALL\"");

        resp = readAT(10000); // tăng timeout để nhận trọn gói
        // Nếu tìm thấy ít nhất một tin
        if (resp.indexOf("+CMGL:") != -1) {
            return resp;
        }
    }

    // Không có tin hoặc không đọc được
    return "";
}

// Xóa tin nhắn theo index (AT+CMGD=index)
bool AIoT_L501::deleteSMS(int index) {
    String resp;
    // Chọn bộ nhớ SIM trước khi xóa
    sendAT("AT+CMGF=1", resp, 2000);
    sendAT("AT+CPMS=\"SM\",\"SM\",\"SM\"", resp, 3000);
    
    String cmd = "AT+CMGD=" + String(index);
    return sendAT(cmd.c_str(), resp, 5000);
}

// Xóa tất cả tin nhắn (AT+CMGD=1,4)
bool AIoT_L501::deleteAllSMS() {
    String resp;
    // Chọn bộ nhớ SIM trước khi xóa
    sendAT("AT+CMGF=1", resp, 2000);
    sendAT("AT+CPMS=\"SM\",\"SM\",\"SM\"", resp, 3000);
    
    return sendAT("AT+CMGD=1,4", resp, 10000);
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

// Gộp AT+MDISCONNECT + AT+MIPCLOSE
bool AIoT_L501::mqttStop() {
    String resp;
    bool success = true;
    
    // Bước 1: Ngắt kết nối MQTT (AT+MDISCONNECT)
    if (!sendAT("AT+MDISCONNECT", resp, 5000)) {
        success = false;
    }
    String resp2 = readAT(5000);
    
    delay(100);
    
    // Bước 2: Đóng kết nối TCP (AT+MIPCLOSE)
    if (!sendAT("AT+MIPCLOSE", resp, 5000)) {
        success = false;
    }
    resp2 = readAT(5000);
    
    return success;
}

// ============================================================================
// TCP/UDP
// ============================================================================

// AT+CIPOPEN=socketId,"TCP","host",port
bool AIoT_L501::tcpConnect(int socketId, const String &host, int port) {
    String resp;
    // Mở mạng nếu cần
    if (!isDataConnected()) {
        if (!netOpen()) {
            // Thử kích hoạt PDP rồi mở mạng
            activatePDP(1);
            if (!netOpen()) return false;
        }
    }

    String cmd = "AT+CIPOPEN=" + String(socketId) + ",\"TCP\",\"" + host + "\"," + String(port);
    if (!sendAT(cmd.c_str(), resp, 15000)) return false;
    String resp2 = readAT(10000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

// AT+CIPOPEN=socketId,"UDP",,
bool AIoT_L501::udpOpen(int socketId) {
    String resp;
    String cmd = "AT+CIPOPEN=" + String(socketId) + ",\"UDP\",,";
    if (!sendAT(cmd.c_str(), resp, 10000)) return false;
    String resp2 = readAT(5000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1);
}

// AT+CIPSEND=socketId rồi gửi data
bool AIoT_L501::tcpSend(int socketId, const String &data) {
    String resp;
    String cmd = "AT+CIPSEND=" + String(socketId);
    
    clearBuffer();
    serial_.println(cmd);
    
    // Chờ dấu ">"
    uint32_t start = millis();
    while (millis() - start < 5000) {
        if (serial_.available()) {
            char c = serial_.read();
            if (c == '>') break;
        }
    }
    
    // Gửi dữ liệu
    serial_.print(data);
    serial_.write(26);  // Ctrl+Z để kết thúc
    
    // Chờ phản hồi
    resp = readAT(10000);
    return resp.indexOf("SUCCESS") != -1 || resp.indexOf("OK") != -1;
}

// AT+CIPSEND=socketId,length rồi gửi data
bool AIoT_L501::tcpSendLen(int socketId, const String &data, int length) {
    String resp;
    String cmd = "AT+CIPSEND=" + String(socketId) + "," + String(length);
    
    clearBuffer();
    serial_.println(cmd);
    
    // Chờ dấu ">"
    uint32_t start = millis();
    while (millis() - start < 5000) {
        if (serial_.available()) {
            char c = serial_.read();
            if (c == '>') break;
        }
    }
    
    // Gửi dữ liệu
    serial_.print(data);
    
    // Chờ phản hồi
    resp = readAT(10000);
    return resp.indexOf("SUCCESS") != -1 || resp.indexOf("OK") != -1;
}

// AT+CIPSEND=socketId,length,"host",port rồi gửi data (UDP)
bool AIoT_L501::udpSend(int socketId, const String &data, const String &host, int port) {
    String resp;
    int len = data.length();
    String cmd = "AT+CIPSEND=" + String(socketId) + "," + String(len) + ",\"" + host + "\"," + String(port);
    
    clearBuffer();
    serial_.println(cmd);
    
    // Chờ dấu ">"
    uint32_t start = millis();
    while (millis() - start < 5000) {
        if (serial_.available()) {
            char c = serial_.read();
            if (c == '>') break;
        }
    }
    
    // Gửi dữ liệu
    serial_.print(data);
    
    // Chờ phản hồi
    resp = readAT(10000);
    return resp.indexOf("SUCCESS") != -1 || resp.indexOf("OK") != -1;
}

// AT+CIPRXGET=2,socketId,length
String AIoT_L501::tcpReceive(int socketId, int length) {
    String resp;
    String cmd;
    
    if (length > 0) {
        cmd = "AT+CIPRXGET=2," + String(socketId) + "," + String(length);
    } else {
        cmd = "AT+CIPRXGET=2," + String(socketId) + ",1024";  // Đọc tối đa 1024 bytes
    }
    
    if (sendAT(cmd.c_str(), resp, 5000)) {
        // Tách lấy dữ liệu từ response
        // Format: +CIPRXGET: SUCCESS,2,socketId,len,remaining,data
        int idx = resp.indexOf("SUCCESS");
        if (idx != -1) {
            // Tìm dấu phẩy cuối cùng trước data
            int commaCount = 0;
            int dataStart = idx;
            while (commaCount < 4 && dataStart < resp.length()) {
                if (resp.charAt(dataStart) == ',') commaCount++;
                dataStart++;
            }
            if (commaCount >= 4) {
                int dataEnd = resp.indexOf("\r\n", dataStart);
                if (dataEnd == -1) dataEnd = resp.length();
                return resp.substring(dataStart, dataEnd);
            }
        }
    }
    return "";
}

// AT+CIPCLOSE=socketId
bool AIoT_L501::tcpClose(int socketId) {
    String resp;
    String cmd = "AT+CIPCLOSE=" + String(socketId);
    if (!sendAT(cmd.c_str(), resp, 5000)) return false;
    String resp2 = readAT(5000);
    return (resp.indexOf("SUCCESS") != -1) || (resp2.indexOf("SUCCESS") != -1) || (resp.indexOf("OK") != -1);
}

// Đóng tất cả socket (1-10)
bool AIoT_L501::tcpCloseAll() {
    bool success = true;
    for (int i = 1; i <= 10; i++) {
        if (!tcpClose(i)) {
            // Bỏ qua lỗi nếu socket chưa mở
        }
    }
    return success;
}

// Kiểm tra trạng thái socket
bool AIoT_L501::isTcpConnected(int socketId) {
    String resp;
    // Theo log của bạn: +CIPCLOSE:<id>,<state> (0=closed, 1=open)
    if (!sendAT("AT+CIPCLOSE?", resp, 5000)) return false;

    String pat = String("+CIPCLOSE:") + String(socketId) + ",";
    int idx = resp.indexOf(pat);
    if (idx == -1) return false;

    int comma = resp.indexOf(',', idx);
    if (comma == -1) return false;

    int eol = resp.indexOf("\r\n", comma + 1);
    String state = (eol != -1) ? resp.substring(comma + 1, eol) : resp.substring(comma + 1);
    state.trim();

    return state == "1";
}

bool AIoT_L501::netIsOpen() {
    String resp;
    if (!sendAT("AT+NETOPEN?", resp, 2000)) return false;
    int idx = resp.indexOf("+NETOPEN:");
    if (idx != -1) {
        int eol = resp.indexOf("\r\n", idx);
        String line = (eol != -1) ? resp.substring(idx, eol) : resp.substring(idx);
        int colon = line.indexOf(':');
        String val = (colon != -1) ? line.substring(colon + 1) : "";
        val.trim();
        return val == "1";
    }
    return resp.indexOf("ALREADY") != -1;
}

bool AIoT_L501::netOpen() {
    if (netIsOpen()) return true;

    clearBuffer();
    serial_.println("AT+NETOPEN");
    String resp = readAT(20000);

    if (resp.indexOf("SUCCESS") != -1) return true;
    if (resp.indexOf("+NETOPEN: 0") != -1) return true;
    if (resp.indexOf("ALREADY") != -1) return true;
    if (resp.indexOf("OK") != -1 && netIsOpen()) return true;

    String tmp;
    sendAT("AT+NETCLOSE", tmp, 3000);

    clearBuffer();
    serial_.println("AT+NETOPEN");
    resp = readAT(20000);
    if (resp.indexOf("SUCCESS") != -1) return true;
    if (resp.indexOf("+NETOPEN: 0") != -1) return true;
    return netIsOpen();
}

bool AIoT_L501::netClose() {
    clearBuffer();
    serial_.println("AT+NETCLOSE");
    String resp = readAT(10000);

    if (resp.indexOf("SUCCESS") != -1) return true;
    if (resp.indexOf("+NETCLOSE: 0") != -1) return true;
    if (resp.indexOf("OK") != -1) return true;

    String tmp;
    if (sendAT("AT+NETOPEN?", tmp, 2000)) {
        int idx = tmp.indexOf("+NETOPEN:");
        if (idx != -1) {
            int eol = tmp.indexOf("\r\n", idx);
            String line = (eol != -1) ? tmp.substring(idx, eol) : tmp.substring(idx);
            int colon = line.indexOf(':');
            String val = (colon != -1) ? line.substring(colon + 1) : "";
            val.trim();
            if (val == "0") return true;
        }
    }
    return false;
}

