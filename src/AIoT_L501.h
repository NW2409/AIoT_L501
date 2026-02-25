#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>

struct SMSMessage {
    String sender;
    String timeBase;
    String content;
    int originalIndexes[8]; // tối đa 8 phần/tin ghép
    int originalCount;
};

class AIoT_L501 {
public:
    /**
     * @brief Khởi tạo đối tượng AIoT_L501
     * @param serial Đối tượng HardwareSerial để giao tiếp (ví dụ: Serial1, Serial2)
     * @param baud Tốc độ baud, mặc định 115200
     */
    AIoT_L501(HardwareSerial &serial, uint32_t baud = 115200, int rxPin = -1, int txPin = -1); // Thêm constructor này

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
    void simPowerOn();
    bool sendAT(const char *cmd, String &response, uint32_t timeout = 2000);

    /**
     * @brief Đọc dữ liệu từ buffer Serial của module
     * @param timeout Thời gian chờ đọc dữ liệu tối đa (ms)
     * @return Chuỗi dữ liệu đọc được từ buffer
     */
    String readAT(uint32_t timeout = 2000);

    /**
     * @brief Gửi lệnh AT không cần chờ phản hồi
     * @param cmd Lệnh AT cần gửi (VD: "AT+CSQ")
     */
    void sendATcmd(const char *cmd);
    bool waitFor(const String &target, uint32_t timeout);


    // ========================================================================
    // Module & SIM Status (Trạng thái Module & SIM)
    // ========================================================================

    /**
     * @brief Kiểm tra SIM đã sẵn sàng chưa
     * @return true nếu SIM sẵn sàng (READY), false nếu không
     */
    bool isSimReady();

    /**
     * @brief Kiểm tra đã đăng ký mạng chưa
     * @return true nếu đã đăng ký mạng, false nếu chưa
     */
    bool isNetworkRegistered();

    /**
     * @brief Kiểm tra kết nối dữ liệu đã kích hoạt chưa
     * @return true nếu đã kết nối, false nếu chưa
     */
    bool isDataConnected();


    /**
     * @brief Lấy chất lượng tín hiệu (CSQ)
     * @return Giá trị RSSI (0-31), -1 nếu thất bại
     */
    int getSignalQuality();

    /**
     * @brief Lấy thông tin module
     * @return Chuỗi thông tin module
     */
    String getModuleInfo();

    /**
     * @brief Lấy địa chỉ IP hiện tại
     * @return Chuỗi địa chỉ IP, rỗng nếu chưa có IP
     */
    String getIPAddress();

    /**
     * @brief Lấy thời gian thực từ module (nếu đã đồng bộ mạng)
     * @return Chuỗi thời gian dạng "yy/MM/dd,hh:mm:ss±zz" hoặc rỗng nếu lỗi
     */
    String getNetworkTime();

    // ========================================================================
    // Network Connection (Kết nối mạng 4G)
    // ========================================================================

    /**
     * @brief Đảm bảo module đã kết nối mạng
     * @param retry Số lần thử lại, mặc định 3
     * @param interval Khoảng cách giữa các lần thử (ms), mặc định 5000ms
     * @return true nếu kết nối thành công, false nếu thất bại
     */
    bool ensureNetwork(uint8_t retry = 3, uint32_t interval = 5000);

    /**
     * @brief Mở kết nối mạng (AT+NETOPEN)
     * @return true nếu thành công, false nếu thất bại
     */
    bool netOpen();

    /**
     * @brief Đóng kết nối mạng (AT+NETCLOSE)
     * @return true nếu thành công, false nếu thất bại
     */
    bool netClose();

    /**
     * @brief Kiểm tra trạng thái NETOPEN
     * @return true nếu đang mở kết nối mạng, false nếu không
     */
    bool netIsOpen(); // mới: kiểm tra trạng thái NETOPEN

    /**
     * @brief Kết nối Internet 4G đầy đủ (CGATT + CGDCONT + CGACT + NETOPEN)
     * @param apn Tên APN
     * @param user Tên người dùng, mặc định rỗng
     * @param pass Mật khẩu, mặc định rỗng
     * @param cid Context ID, mặc định 1
     * @return true nếu kết nối thành công, false nếu thất bại
     */
    bool connectInternet4G(const String &apn, const String &user = "", const String &pass = "", int cid = 1);
    /**
     * @brief Kết nối Internet 4G cho module Quectel (QICSGP + NETOPEN)
     * @param apn Tên APN
     * @param user Tên người dùng, mặc định rỗng
     * @param pass Mật khẩu, mặc định rỗng
     * @param cid Context ID, mặc định 1
     * @param contextType Loại kết nối (1: IPV4, 2: IPV4V6, 3: IPV6), mặc định 1
     * @param auth Loại xác thực (0: none, 1: PAP, 2: CHAP), mặc định 1
     * @return true nếu kết nối thành công, false nếu thất bại
     */
    bool connectInternet4G_L501(const String &apn, const String &user = "", const String &pass = "", int cid = 1, int contextType = 1, int auth = 1);

    // ========================================================================
    // SMS Functions (Nhắn tin SMS)
    // ========================================================================

    /**
     * @brief Gửi tin nhắn SMS
     * @param phone Số điện thoại người nhận
     * @param message Nội dung tin nhắn
     * @return true nếu gửi thành công, false nếu thất bại
     */
    bool sendSMS(const String &phone, const String &message);

    /**
     * @brief Đọc tin nhắn SMS theo chỉ số
     * @param index Chỉ số tin nhắn trong bộ nhớ
     * @return Nội dung tin nhắn, rỗng nếu không tìm thấy
     */
    String readSMS(int index);

    /**
     * @brief Liệt kê tất cả tin nhắn SMS
     * @return Danh sách tin nhắn, rỗng nếu không có
     */
    String listAllSMS();

    /**
     * @brief Xóa tin nhắn SMS theo chỉ số (AT+CMGD)
     * @param index Chỉ số tin nhắn cần xóa
     * @return true nếu xóa thành công, false nếu thất bại
     */
    bool deleteSMS(int index);

    /**
     * @brief Xóa tất cả tin nhắn SMS
     * @return true nếu xóa thành công, false nếu thất bại
     */
    bool deleteAllSMS();

    /**
     * @brief Giải mã chuỗi UCS2 sang UTF8
     * @param hexInput Chuỗi đầu vào dạng hex
     * @return Chuỗi đã giải mã sang UTF8
     */
    String decodeUCS2ToUTF8(const String& hexInput);

    /**
     * @brief Lấy danh sách tin nhắn hợp nhất
     * @param outArray Mảng đầu ra để lưu danh sách tin nhắn
     * @param maxCount Số lượng tin nhắn tối đa có thể lưu
     * @return Số lượng tin nhắn đã được lưu vào mảng
     */
    int getMergedSMSList(SMSMessage* outArray, int maxCount);

    // ========================================================================
    // Call Functions (Cuộc gọi)
    // ========================================================================

    /**
     * @brief Thực hiện cuộc gọi
     * @param phone Số điện thoại cần gọi
     * @return true nếu gọi thành công, false nếu thất bại
     */
    bool call(const String &phone);

    /**
     * @brief Kết thúc cuộc gọi
     * @return true nếu thành công, false nếu thất bại
     */
    bool hangUp();

    /**
     * @brief Trả lời cuộc gọi đến
     * @return true nếu thành công, false nếu thất bại
     */
    bool answerCall();

    // ========================================================================
    // MQTT Functions (Giao thức MQTT)
    // ========================================================================

    /**
     * @brief Cấu hình MQTT client (AT+MCONFIG)
     * @param clientId ID của client MQTT
     * @param username Tên đăng nhập, mặc định rỗng
     * @param password Mật khẩu, mặc định rỗng
     * @return true nếu thành công, false nếu thất bại
     */
    bool mqttConfig(const String &clientId, const String &username = "", const String &password = "");
    //bool mqttConfig(const String &clientId, const String &username, const String &password, String &resp);  // Thêm dòng này

    /**
     * @brief Thiết lập server MQTT và kết nối TCP (AT+MIPSTART)
     * @param address Địa chỉ broker (VD: "broker.emqx.io")
     * @param port Cổng kết nối, thường là 1883
     * @param version Phiên bản MQTT: 3 = MQTT 3.1, 4 = MQTT 3.1.1
     * @return true nếu thành công, false nếu thất bại
     */
    bool mqttSetServer(const String &address, int port, int version = 4);

    /**
     * @brief Kết nối đến broker MQTT (AT+MCONNECT)
     * @param cleanSession 1 = Clean session, 0 = Persistent session
     * @param keepalive Thời gian keep-alive (giây), mặc định 60
     * @return true nếu kết nối thành công, false nếu thất bại
     */
    bool mqttConnect(int cleanSession= 1 , int keepalive = 60);

    /**
     * @brief Gửi tin nhắn MQTT ngắn (AT+MPUB)
     * @param topic Topic cần gửi
     * @param payload Nội dung tin nhắn
     * @param qos Chất lượng dịch vụ: 0, 1, hoặc 2
     * @param retain 1 = Retain message, 0 = Không retain
     * @return true nếu gửi thành công, false nếu thất bại
     */
    bool mqttPublish(const String &topic, const String &payload, int qos = 0, int retain = 0);

    /**
     * @brief Gửi tin nhắn MQTT dài hoặc JSON (AT+MPUBEX)
     * @param topic Topic cần gửi
     * @param payload Nội dung tin nhắn (có thể là JSON)
     * @param qos Chất lượng dịch vụ: 0, 1, hoặc 2
     * @param retain 1 = Retain message, 0 = Không retain
     * @return true nếu gửi thành công, false nếu thất bại
     */
    bool mqttPublishEx(const String &topic, const String &payload, int qos = 0, int retain = 0);

    /**
     * @brief Đăng ký nhận tin nhắn từ topic (AT+MSUB)
     * @param topic Topic cần đăng ký
     * @param qos Chất lượng dịch vụ: 0, 1, hoặc 2
     * @return true nếu đăng ký thành công, false nếu thất bại
     */
    bool mqttSubscribe(const String &topic, int qos = 0);

    /**
     * @brief Hủy đăng ký topic (AT+MUNSUB)
     * @param topic Topic cần hủy đăng ký
     * @return true nếu hủy thành công, false nếu thất bại
     */
    bool mqttUnsubscribe(const String &topic);

    /**
     * @brief Ngắt kết nối MQTT (AT+MDISCONNECT)
     * @return true nếu thành công, false nếu thất bại
     */
    bool mqttDisconnect();

    /**
     * @brief Đóng kết nối TCP MQTT (AT+MIPCLOSE)
     * @return true nếu thành công, false nếu thất bại
     */
    bool mqttClose();

    /**
     * @brief Ngắt kết nối MQTT và đóng TCP (AT+MDISCONNECT + AT+MIPCLOSE)
     * @return true nếu thành công, false nếu thất bại
     */
    bool mqttStop();

    /**
     * @brief Lấy trạng thái kết nối MQTT
     * @return Mã trạng thái (0 = chưa kết nối, 1 = đang kết nối, ...)
     */
    int mqttStatus();

    // ========================================================================
    // MQTT Receive & Parse (Nhận và tách dữ liệu MQTT)
    // ========================================================================

    /**
     * @brief Đọc tin nhắn MQTT từ buffer
     * @param timeout Thời gian chờ đọc (ms)
     * @return Chuỗi dữ liệu MQTT (VD: +MSUB:"topic",8 bytes,"payload"), rỗng nếu không có
     */
    String mqttReceive(uint32_t timeout = 5000);

    /**
     * @brief Tách lấy tên topic từ dữ liệu MQTT
     * @param data Chuỗi dữ liệu MQTT (từ mqttReceive)
     * @return Tên topic, rỗng nếu không tìm thấy
     */
    // String mqttGetTopic(const String &data);

    // /**
    //  * @brief Tách lấy payload từ dữ liệu MQTT
    //  * @param data Chuỗi dữ liệu MQTT (từ mqttReceive)
    //  * @return Nội dung payload, rỗng nếu không tìm thấy
    //  */
    // String mqttGetPayload(const String &data);

    // /**
    //  * @brief Tách lấy độ dài payload từ dữ liệu MQTT
    //  * @param data Chuỗi dữ liệu MQTT (từ mqttReceive)
    //  * @return Độ dài payload (bytes), 0 nếu không tìm thấy
    //  */
    // int mqttGetLength(const String &data);

    // ========================================================================
    // TCP Functions (Kết nối TCP)
    // ========================================================================

    /**
     * @brief Mở kết nối TCP đến server
     * @param socketId ID socket (1-10)
     * @param host Địa chỉ server (IP hoặc domain)
     * @param port Cổng kết nối
     * @return true nếu kết nối thành công, false nếu thất bại
     */
    bool tcpConnect(int socketId, const String &host, int port);
    /**
     * @brief Gửi dữ liệu qua TCP
     * @param socketId ID socket
     * @param data Dữ liệu cần gửi
     * @return true nếu gửi thành công, false nếu thất bại
     */
    bool tcpSend(int socketId, const String &data);
    /**
     * @brief Gửi dữ liệu nhỏ qua TCP (dưới 512 byte)
     * @param socketId ID socket
     * @param data Dữ liệu cần gửi
     * @return true nếu gửi thành công, false nếu thất bại
     */
    bool tcpSendSmall(int socketId, const String &data);

    /**
     * @brief Gửi dữ liệu qua TCP với độ dài cố định
     * @param socketId ID socket
     * @param data Dữ liệu cần gửi
     * @param length Độ dài dữ liệu
     * @return true nếu gửi thành công, false nếu thất bại
     */
    
    bool tcpSendLen(int socketId, const String &data, int length);
    /**
     * @brief Nhận dữ liệu từ socket
     * @param socketId ID socket
     * @param length Số byte muốn đọc (0 = đọc tất cả)
     * @return Dữ liệu nhận được, rỗng nếu không có
     */
    String tcpReceive(int socketId, int length = 0);
    /**
     * @brief Kiểm tra số byte còn lại trong buffer TCP của socket
     * @param socketId ID socket
     * @return Số byte còn lại trong buffer, 0 nếu không có
     */
    int tcpAvailable(int socketId);

    /**
     * @brief Đóng kết nối socket
     * @param socketId ID socket cần đóng
     * @return true nếu đóng thành công, false nếu thất bại
     */
    bool tcpClose(int socketId);

    /**
     * @brief Đóng tất cả kết nối socket
     * @return true nếu đóng thành công, false nếu thất bại
     */
    bool tcpCloseAll();
    /**
     * @brief Kiểm tra trạng thái kết nối socket
     * @param socketId ID socket
     * @return true nếu đang kết nối, false nếu không
     */
    bool isTcpConnected(int socketId);
    // /**
    //  * @brief Mở kết nối UDP
    //  * @param socketId ID socket (1-10)
    //  * @return true nếu mở thành công, false nếu thất bại
    //  */
    
    // bool udpOpen(int socketId);

    // /**
    //  * @brief Gửi dữ liệu qua UDP đến địa chỉ cụ thể
    //  * @param socketId ID socket
    //  * @param data Dữ liệu cần gửi
    //  * @param host Địa chỉ server đích
    //  * @param port Cổng đích
    //  * @return true nếu gửi thành công, false nếu thất bại
    //  */
    // bool udpSend(int socketId, const String &data, const String &host, int port);

     /**
     * @brief Ping một địa chỉ host qua mạng 4G
     * @param host Địa chỉ host (domain hoặc IP)
     * @param count Số lần ping (1-100)
     * @param size Kích thước gói dữ liệu (32-256)
     * @param wait Thời gian chờ mỗi ping (1-255)
     * @return Chuỗi kết quả ping trả về từ module
     */
    String ping(const String &host, int count = 4, int size = 32, int wait = 3);

    /**
     * @brief Đặt thời gian thực cho module (theo chuẩn AT+CCLK)
     * @param time Chuỗi thời gian dạng "yy/MM/dd,hh:mm:ss+zz"
     * @return true nếu thành công, false nếu lỗi
     */
    bool setNetworkTime(const String &time);
    /**
     * @brief Đóng tất cả PDP context và socket khi khởi động để tránh lỗi kết nối
     */
    void cleanStart();

    // ========================================================================
    // HTTP 
    // ========================================================================
    /**
     * @brief Open HTTP service on module (AT$HTTPOPEN)
     * @return true if module replies OK
     */
    bool httpOpen();

    /**
     * @brief Close HTTP service on module (AT$HTTPCLOSE)
     * @return true if module replies OK
     */
    bool httpClose();

    /**
     * @brief Set HTTP parameters (AT$HTTPPARA)
     * @param url Full URL string including scheme and path (e.g. "http://host/path")
     * @param port TCP port (0 means use default from URL)
     * @param ssl 0 = no SSL, 1 = SSL
     * @return true if command accepted
     */
    bool httpSetPara(const String &url, int port = 0, int ssl = 0);

    /**
     * @brief Set a single HTTP request header (AT$HTTPRQH)
     * @param name Header name (e.g. "Content-Type")
     * @param value Header value (e.g. "application/json")
     * @return true if command accepted
     */
    bool httpSetHeader(const String &name, const String &value);

    /**
     * @brief Notify module that application will send POST data (AT$HTTPDATA)
     * @param length Number of bytes to send
     * @param timeout Time in ms the module will wait for the data
     * @return true if module ready to receive data (prompt or OK)
     */
    bool httpDataBegin(size_t length, uint32_t timeout = 10000);

    /**
     * @brief Send previously prepared HTTP data and trigger send sequence (AT$HTTPSEND)
     * @param data Payload to write to module's data buffer (module-specific flow)
     * @return true if send command accepted
     */
    bool httpSendData(const String &data);

    /**
     * @brief Execute HTTP action (AT$HTTPACTION)
     * @param method 0=GET, 1=POST, 3=POST_EX (see modem guide)
     * @param timeout Milliseconds to wait for $HTTPRECV responses and body
     * @return Raw modem response (may include $HTTPRECV:DATA blocks and HTTP headers/body)
     */
    String httpAction(int method, uint32_t timeout = 15000);

    /**
     * @brief Convenience: perform HTTP GET
     * @param url Full URL
     * @param timeout Milliseconds to wait for response
     * @return Raw modem output for the GET (headers+body blocks)
     */
    String httpGet(const String &url, uint32_t timeout = 15000);

    /**
     * @brief Convenience: perform HTTP POST with payload
     * @param url Full URL
     * @param payload Body to POST
     * @param timeout Milliseconds to wait for response
     * @return Raw modem output for the POST (headers+body blocks)
     */
    String httpPost(const String &url, const String &payload, uint32_t timeout = 20000);
    /**
     * @brief Đổi baudrate cho cả module SIM (AT+IPR) và UART ESP32
     * @param baud Baudrate mới (ví dụ 9600, 115200)
     * @param timeout Thời gian chờ OK cho AT+IPR (ms), mặc định 3000ms
     * @return true nếu đổi thành công và bắt tay lại "AT" OK, false nếu thất bại (tự rollback UART)
     */

    bool setBaudrate(uint32_t baud, uint32_t timeout = 3000);

    /**
     * @brief Lấy baudrate hiện tại của SDK
     */
    uint32_t getBaudrate() const;
        /**
    * @brief Tắt nguồn module
     */
    void powerOff();
   
private:
    HardwareSerial &serial_;
    uint32_t baud_;
    int rxPin_ = -1, txPin_ = -1; // Thêm biến lưu chân
    void clearBuffer();
};
