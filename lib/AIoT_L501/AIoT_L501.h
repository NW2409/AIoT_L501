#pragma once
#include <Arduino.h>

class AIoT_L501 {
public:
    /**
     * @brief Khởi tạo đối tượng AIoT_L501
     * @param serial Đối tượng HardwareSerial để giao tiếp (ví dụ: Serial1, Serial2)
     * @param baud Tốc độ baud, mặc định 115200
     */
    AIoT_L501(HardwareSerial &serial, uint32_t baud = 115200);

    /**
     * @brief Khởi động kết nối Serial
     */
    void begin();

    /**
     * @brief Khởi tạo module, kiểm tra AT, SIM, Sóng
     * @param timeout Thời gian chờ khởi tạo tối đa (ms), mặc định 10000ms
     * @return true nếu khởi tạo thành công (Module OK, SIM OK), false nếu thất bại
     */
    bool init(unsigned long timeout = 10000);

    // ========================================================================
    // AT Command Functions (Giao tiếp AT)
    // ========================================================================
    
    /**
     * @brief Gửi lệnh AT và chờ phản hồi
     * @param cmd Lệnh AT cần gửi (VD: "AT+CSQ")
     * @param response Chuỗi tham chiếu để lưu phản hồi từ module
     * @param timeout Thời gian chờ phản hồi tối đa (ms)
     * @return true nếu nhận được "OK", false nếu "ERROR" hoặc timeout
     */
    bool sendAT(const char *cmd, String &response, uint32_t timeout = 2000);

    /**
     * @brief Đọc phản hồi từ module trong khoảng thời gian timeout
     * @param timeout Thời gian chờ đọc tối đa (ms)
     * @return Chuỗi dữ liệu đọc được
     */
    String readAT(uint32_t timeout = 2000);

    // ========================================================================
    // Status & Info (Thông tin và Trạng thái)
    // ========================================================================
    
    /**
     * @brief Kiểm tra trạng thái SIM (AT+CPIN?)
     * @return true nếu SIM sẵn sàng (READY), false nếu chưa nhận hoặc lỗi
     */
    bool isSimReady();

    /**
     * @brief Kiểm tra đã đăng ký mạng (AT+CREG? / AT+CGREG?)
     * @return true nếu đã đăng ký mạng (Home hoặc Roaming), false nếu chưa
     */
    bool isNetworkRegistered();

    /**
     * @brief Lấy số IMEI của module (AT+CGSN)
     * @return Chuỗi IMEI
     */
    String getIMEI();

    /**
     * @brief Lấy chất lượng tín hiệu rSSI (AT+CSQ)
     * @return Giá trị RSSI (0-31), 99 là lỗi/không rõ
     */
    int getSignalQuality();

    /**
     * @brief Lấy thông tin phiên bản module (ATI)
     * @return Chuỗi thông tin module
     */
    String getModuleInfo();

    // ========================================================================
    // Network Connectivity (Kết nối Mạng cơ bản)
    // ========================================================================

    /**
     * @brief Đảm bảo module đã kết nối mạng, tự động thử lại nhiều lần
     * @param retry Số lần thử lại tối đa (mặc định 3)
     * @param interval Khoảng thời gian nghỉ giữa các lần thử (ms)
     * @return true nếu kết nối thành công, false nếu thất bại
     */
    bool ensureNetwork(uint8_t retry = 3, uint32_t interval = 5000);

    /**
     * @brief Cấu hình APN cho GPRS (AT+CGDCONT)
     * @param apn Tên APN (VD: "v-internet")
     * @param user User đăng nhập (nếu có)
     * @param pass Password đăng nhập (nếu có)
     * @return true nếu cấu hình thành công
     */
    bool attachGPRS(const String &apn, const String &user = "", const String &pass = "");

    /**
     * @brief Kích hoạt PDP Context (AT+CGACT)
     * @param cid Context ID (thường là 1)
     * @return true nếu kích hoạt thành công
     */
    bool activatePDP(int cid = 1);

    /**
     * @brief Ngắt PDP Context
     * @param cid Context ID
     * @return true nếu ngắt thành công
     */
    bool deactivatePDP(int cid = 1);

    /**
     * @brief Hàm tiện ích kết nối Internet 4G (Gộp Attach + Activate)
     */
    bool connectInternet4G(const String &apn, const String &user = "", const String &pass = "", int cid = 1);

    /**
     * @brief Lấy địa chỉ IP hiện tại (AT+CGPADDR)
     * @return Chuỗi IP
     */
    String getIPAddress();

    /**
     * @brief Kiểm tra xem kết nối dữ liệu packet có đang hoạt động không
     * @return true nếu connected
     */
    bool isDataConnected();

    // ========================================================================
    // SMS Functions (Tin nhắn)
    // ========================================================================

    /**
     * @brief Gửi tin nhắn SMS
     * @param phone Số điện thoại người nhận
     * @param message Nội dung tin nhắn
     * @return true nếu gửi thành công
     */
    bool sendSMS(const String &phone, const String &message);

    /**
     * @brief Đọc tin nhắn tại vị trí index
     * @param index Vị trí tin nhắn trong bộ nhớ
     * @return Nội dung tin nhắn (hoặc chuỗi raw)
     */
    String readSMS(int index);

    /**
     * @brief Liệt kê tất cả tin nhắn (AT+CMGL="ALL")
     * @return Danh sách tin nhắn format kêt hợp
     */
    String listAllSMS();

    // --- Advanced SMS ---

    /**
     * @brief Xóa tin nhắn (AT+CMGD)
     * @param index Vị trí tin nhắn
     * @param flag 0: Xóa 1 tin, 4: Xóa tất cả tin
     * @return true nếu xóa thành công
     */
    bool deleteSMS(int index, int flag = 0);

    /**
     * @brief Lưu tin nhắn vào bộ nhớ (AT+CMGW)
     * @param phone Số điện thoại
     * @param message Nội dung
     * @return Index của tin nhắn vừa lưu, -1 nếu lỗi
     */
    int saveSMS(const String &phone, const String &message);

    /**
     * @brief Gửi tin nhắn đã lưu trong bộ nhớ (AT+CMSS)
     * @param index Vị trí tin nhắn
     * @param phone Số điện thoại (tùy chọn, nếu không gửi theo số đã lưu)
     * @return true nếu gửi thành công
     */
    bool sendStoredSMS(int index, const String &phone = "");

    /**
     * @brief Cài đặt số trung tâm tin nhắn (AT+CSCA)
     * @param sc_number Số trung tâm
     * @return true nếu thành công
     */
    bool setServiceCenter(const String &sc_number);

    /**
     * @brief Cài đặt tham số Text Mode (AT+CSMP)
     * @param fo First Octet
     * @param vp Validity Period
     * @param pid Protocol Identifier
     * @param dcs Data Coding Scheme
     */
    bool setTextModeParams(int fo, int vp, int pid, int dcs);

    /**
     * @brief Cài đặt báo tin nhắn mới tới UART (AT+CNMI)
     */
    bool setNewMessageIndication(int mode, int mt, int bm, int ds, int bfr);

    /**
     * @brief Chọn dịch vụ tin nhắn (AT+CSMS)
     * @param service Loại dịch vụ (0: 3GPP 27.005, 1: 3GPP2)
     */
    bool selectMessageService(int service);

    /**
     * @brief Chọn bộ nhớ lưu trữ SMS (AT+CPMS)
     * @param mem1 Bộ nhớ đọc/xóa (VD: "SM", "ME")
     * @param mem2 Bộ nhớ ghi/gửi
     * @param mem3 Bộ nhớ nhận
     */
    bool selectStorage(const String &mem1, const String &mem2, const String &mem3);

    // ========================================================================
    // Call Functions (Cuộc gọi)
    // ========================================================================

    bool call(const String &phone); 
    bool hangUp();
    bool answerCall();

    // --- Advanced Call ---

    /**
     * @brief Thực hiện cuộc gọi khẩn cấp (AT*DIALE)
     */
    bool callEmergency();

    /**
     * @brief Liệt kê các cuộc gọi hiện tại (AT+CLCC)
     * @return Danh sách cuộc gọi raw response
     */
    String listCalls();

    /**
     * @brief Chuyển tiếp cuộc gọi (AT+CCFC)
     * @param reason Lý do (0: Unconditional, 1: Busy...)
     * @param mode 0: Disable, 1: Enable, 3: Registration...
     * @param number Số điện thoại chuyển tới
     */
    bool forwardCall(int reason, int mode, const String &number = "", int classx = 7);

    /**
     * @brief Kích hoạt cuộc gọi chờ (AT+CCWA)
     * @param mode 0: Disable, 1: Enable
     */
    bool callWaiting(int mode);

    /**
     * @brief Điều khiển giữ cuộc gọi (AT+CHLD)
     * @param n Lệnh điều khiển (0, 1, 1x, 2, 2x, 3...)
     */
    bool callHold(int n);

    /**
     * @brief Hạn chế dịch vụ SIM (AT+CRSM)
     */
    bool restrictSIM(int command, int fileid, int p1, int p2, int p3, const String &data = "");

    // ========================================================================
    // MQTT Functions (Giao thức MQTT)
    // ========================================================================

    /**
     * @brief Cấu hình Client MQTT (AT+MCONFIG)
     * @param clientId ID định danh Client
     * @param username Tên đăng nhập
     * @param password Mật khẩu
     */
    bool mqttConfig(const String &clientId, const String &username = "", const String &password = "");

    /**
     * @brief Thiết lập Server MQTT (AT+MIPSTART)
     * @param address Địa chỉ Server (IP hoặc Domain)
     * @param port Cổng (thường là 1883)
     */
    bool mqttSetServer(const String &address, int port, int version = 4);

    /**
     * @brief Kết nối tới Broker (AT+MCONNECT)
     * @param cleanSession 0 hoặc 1
     * @param keepalive Thời gian keepalive (giây)
     */
    bool mqttConnect(int cleanSession = 1, int keepalive = 60);

    /**
     * @brief Publish message (AT+MPUB)
     * @param topic Chủ đề
     * @param payload Nội dung
     * @param qos Quality of Service (0, 1, 2)
     * @param retain Retain flag (0, 1)
     */
    bool mqttPublish(const String &topic, const String &payload, int qos = 0, int retain = 0);

    bool mqttSubscribe(const String &topic, int qos = 0);
    bool mqttUnsubscribe(const String &topic);
    bool mqttDisconnect();
    bool mqttClose();
    int mqttStatus();
    String mqttReceive(uint32_t timeout = 5000);

    // ========================================================================
    // HTTP/HTTPS Functions
    // ========================================================================

    /**
     * @brief Khởi động dịch vụ HTTP (AT$HTTPOPEN)
     */
    bool httpBegin();

    /**
     * @brief Đóng dịch vụ HTTP (AT$HTTPCLOSE)
     */
    void httpStop();

    /**
     * @brief Thực hiện HTTP GET
     * @param url Đường dẫn đầy đủ (VD: http://example.com/api)
     * @return Chuỗi phản hồi từ server
     */
    String httpGET(const String &url);

    /**
     * @brief Thực hiện HTTP POST (Cơ bản)
     * @param url Đường dẫn đầy đủ
     * @param contentType Loại dữ liệu (VD: "application/json")
     * @param data Dữ liệu body
     * @return Chuỗi phản hồi
     */
    String httpPOST(const String &url, const String &contentType, const String &data);
    
    void parseUrl(const String &url, String &host, int &port, int &isHttps);
    String readHttpBody(uint32_t timeout);

    // --- Advanced HTTP (L501 Specific) ---

    /**
     * @brief Cài đặt Header tùy chỉnh (AT$HTTPRQH)
     * @param name Tên header (VD: "Authorization")
     * @param value Giá trị header
     */
    bool httpSetHeader(const String &name, const String &value);

    /**
     * @brief Cài đặt vị trí lưu phản hồi (AT$HTTPTYPE)
     * @param type 0: Lưu vào RAM (Buffer), 1: Lưu vào File System
     */
    bool httpSetSaveLocation(int type);

    /**
     * @brief Đọc dữ liệu HTTP đã tải về từ File/Buffer (AT$HTTPREAD)
     * @param len Độ dài cần đọc
     */
    String httpReadFromFile(uint32_t len, uint32_t timeout = 5000);

    /**
     * @brief Thực hiện HTTP POST chuẩn (AT$HTTPDATA + AT$HTTPSEND)
     * Dùng cho dữ liệu lớn hoặc luồng chuẩn của L501
     */
    String httpPostStandard(const String &url, const String &contentType, const String &data);

    // ========================================================================
    // USSD & Utilities
    // ========================================================================

    bool sendUSSD(const String &ussd); // Gửi lệnh USSD (VD: *101#)
    bool cancelUSSD(); // Hủy phiên USSD
    bool sleep(bool enable); // Bật/Tắt chế độ ngủ tiết kiệm pin (AT+CSCLK)

    // Audio / DTMF (Chỉ khai báo interface - Chưa implement)
    bool setVolume(int level);
    int getVolume();
    bool sendDTMF(const String &dtmf);

    // ========================================================================
    // Raw Socket (TCP/UDP) - Standard AT
    // ========================================================================

    bool socketBegin();
    bool socketConnect(const char* type, const char* host, int port); // Kết nối socket đơn
    bool socketSend(const String &data); // Gửi dữ liệu qua socket đơn
    String socketRead(uint32_t timeout = 5000);
    bool socketClose();
    bool socketStatus();

    // ========================================================================
    // Advanced TCP/IP (L501 Specific dialect)
    // ========================================================================

    /**
     * @brief Cấu hình APN cho Stack TCP/IP riêng (AT+QICSGP)
     * @param contextID ID ngữ cảnh (1-16)
     * @param contextType Loại IP (1: IPV4, 2: IPV6)
     * @param apn Tên APN
     */
    bool netConfigAPN(int contextID, int contextType, const String &apn, const String &user = "", const String &pass = "", int auth = 0);
    
    bool netOpen(); // Mở Network Stack (AT+NETOPEN)
    bool netClose(); // Đóng Network Stack (AT+NETCLOSE)
    bool netSetDNS(const String &dns1, const String &dns2 = ""); // Cài đặt DNS (AT+NETDNS)

    /**
     * @brief Mở kết nối Socket Đa kênh (AT+CIPOPEN)
     * @param connectID ID kết nối (0-11)
     * @param type "TCP" hoặc "UDP"
     * @param host Địa chỉ host
     * @param port Cổng
     */
    bool socketOpen(int connectID, const String &type, const String &host, int port);
    
    bool socketManualGet(int mode); // Cấu hình nhận dữ liệu thụ động (AT+CIPRXGET)
    bool socketTransMode(int mode); // Cấu hình chế độ truyền Transparent (AT+CIPMODE)

    String ping(const String &host); // Lệnh Ping (AT+MPING)
    bool serverStart(int port); // Khởi động TCP Server (AT+SERVERSTART)
    bool serverStop(); // Dừng TCP Server
    
    String dnsLookup(const String &domain); // Phân giải tên miền (AT+MDNSGIP)
    String dataUsage(); // Tra cứu lưu lượng data (AT+USEDDATA)

    // ========================================================================
    // File System (AT+MFS...)
    // ========================================================================

    /**
     * @brief Lưu nội dung vào file (Wrapper cho Create + Write)
     * @param filename Tên file
     * @param content Nội dung chuỗi
     */
    bool fileSave(const String &filename, const String &content);
    String fileLoad(const String &filename); // Đọc nội dung file
    bool fileDel(const String &filename); // Xóa file

    bool fsCD(const String &path); // Chuyển thư mục (AT+MFSCD)
    bool fsMkdir(const String &path); // Tạo thư mục (AT+MFSMKDIR)
    String fsLs(); // Liệt kê file/thư mục (AT+MFSLS)
    bool fsRename(const String &oldName, const String &newName);
    bool fsCreate(const String &filename); // Tạo file rỗng (AT+MFSCREATE)
    String fsMem(); // Kiểm tra bộ nhớ (AT+MFSMEM)
    bool fsCopy(const String &src, const String &dest); // Copy file (AT+MFSCOPY)
    
    /**
     * @brief Ghi dữ liệu vào file tại offset (AT+MFSWRITE)
     */
    bool fsWrite(const String &filename, uint32_t offset, const String &data);

    // ========================================================================
    // Utilities & System
    // ========================================================================

    String getClock(); // Lấy thời gian thực (AT+CCLK?)
    bool syncNTP(const String &server, int timezone); // Đồng bộ NTP (AT+NTP)
    
    /**
     * @brief Cấu hình SSL/TLS (AT+CSSLCFG)
     */
    bool sslConfig(int ssl_ctx_index, int verify_mode, int security_level);

    bool switchToCommandMode(); // Chuyển sang chế độ lệnh (+++)
    bool switchToDataMode();    // Chuyển sang chế độ data (ATO)
    bool setAutoAnswer(int rings); // Tự động trả lời (ATS0)
    bool factoryReset();        // Khôi phục gốc (AT&F)

private:
    HardwareSerial &serial_;
    uint32_t baud_;
};