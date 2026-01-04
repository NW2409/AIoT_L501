
#include <Arduino.h>
#include <AIoT_L501.h>

#define RX_PIN    16
#define TX_PIN    17
#define RST_PIN   4  

AIoT_L501 aiot(Serial2, 115200);
const int MAX_RETRY = 3;  
int errorCount = 0;      

String extractBody(const String &raw) {
  int idx = raw.lastIndexOf("$HTTPRECV:DATA,");
  if (idx == -1) return "";
  int start = raw.indexOf('{', idx);
  if (start == -1) start = raw.indexOf('\n', idx);
  if (start == -1) return "";
  String body = raw.substring(start);
  body.trim();
  return body;
}

void sendATSilent(String cmd) {
  Serial2.println(cmd);
  delay(500);
  while(Serial2.available()) Serial2.read();
}

void hardResetSIM() {
  Serial.println("\n[SYSTEM] >>> Đang thực hiện HARD RESET module SIM...");
  digitalWrite(RST_PIN, HIGH); 
  delay(100);
  digitalWrite(RST_PIN, LOW);  
  delay(500);                 
  digitalWrite(RST_PIN, HIGH);
  
  Serial.println("[SYSTEM] >>> Đã Reset xong. Đợi module khởi động (12s)...");
  delay(12000); 
}

bool connectNetwork() {
  while (true) {
    Serial.println("\n>> Đang thiết lập kết nối 4G...");
    aiot.begin();
    Serial.println("Làm mới trạng thái SIM (CFUN=0)...");
    sendATSilent("AT+CFUN=0"); 
    delay(2000); 

    Serial.println("Bật lại module (CFUN=1)...");
    sendATSilent("AT+ENPWRSAVE=0"); 
    sendATSilent("AT+CFUN=1"); 
    delay(3000); 
    if (aiot.init(10000) && aiot.connectInternet4G("v-internet")) {
      Serial.println("[OK] Kết nối mạng THÀNH CÔNG.");
      errorCount = 0; 
      return true;    
    }

    Serial.println("[LỖI] Không thấy SIM hoặc sóng yếu.");
    Serial.println("[AUTO-FIX] Reset cứng và thử lại sau 5s...");
    hardResetSIM();
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);
  delay(1000);
  Serial.println("\n=== HỆ THỐNG KHỞI ĐỘNG ===");
  connectNetwork();
}

void loop() {
  unsigned long tStart, tDuration; 

  Serial.println("\n>> [GET] http://postman-echo.com/get...");
  tStart = millis(); 
  String getRaw = aiot.httpGet("http://postman-echo.com/get?device=ESP32", 15000);
  tDuration = millis() - tStart; 
  
  if (extractBody(getRaw).length() > 0) {
    Serial.print(">>> GET OK. Thời gian: ");
    Serial.print(tDuration);
    Serial.println(" ms");
    errorCount = 0; 
  } else {
    Serial.print("[LỖI] GET thất bại. Thời gian: ");
    Serial.print(tDuration);
    Serial.println(" ms");
    errorCount++;
  }

  delay(2000);
  Serial.println(">> [POST] http://postman-echo.com/post...");
  String payload = "{\"uptime\":" + String(millis()) + "}";
  
  tStart = millis(); 
  String postRaw = aiot.httpPost("http://postman-echo.com/post", payload, 15000);
  tDuration = millis() - tStart; 
  
  if (extractBody(postRaw).length() > 0) {
    Serial.print(">>> POST OK. Thời gian: ");
    Serial.print(tDuration);
    Serial.println(" ms");
    errorCount = 0;
  } else {
    Serial.print("[LỖI] POST thất bại. Thời gian: ");
    Serial.print(tDuration);
    Serial.println(" ms");
    errorCount++;
  }

  if (errorCount >= MAX_RETRY) {
    Serial.println("\n[CẢNH BÁO] Mất kết nối.");
    Serial.println("[AUTO-FIX] Hệ thống sẽ Reset SIM và kết nối lại...");
    
    aiot.netClose();  
    hardResetSIM();  
    connectNetwork(); 
  }
  Serial.println("... Đợi 15s trước khi lặp lại ...");
  delay(15000);
}
