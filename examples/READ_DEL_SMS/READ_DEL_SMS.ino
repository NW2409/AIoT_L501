#include <Arduino.h>
#include <AIoT_L501.h>

#define L501_RX 16
#define L501_TX 17

AIoT_L501 aiot(Serial2, 115200, L501_RX, L501_TX);
SMSMessage smsArr[20];

void printSMSList(SMSMessage* arr, int count) {
    if (count > 0) {
        Serial.printf("[+] Có %d tin nhắn hoàn chỉnh:\n", count);
        Serial.println("═══════════════════════════════════════════════════");
        for (int i = 0; i < count; i++) {
            Serial.printf("Tin %02d │ Từ: %s\n", i + 1, arr[i].sender.c_str());
            Serial.println("Nội dung:");
            Serial.println(arr[i].content);
            Serial.print("Các index gốc: ");
            for (int j = 0; j < arr[i].originalCount; j++) {
                Serial.print(arr[i].originalIndexes[j]);
                if (j < arr[i].originalCount - 1) Serial.print(", ");
            }
            Serial.println();
            Serial.println("────────────────────────────────────────────");
        }
        Serial.println("═══════════════════════════════════════════════════");
    } else {
        Serial.println("[-] Không có tin nhắn hoặc đọc lỗi.");
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    delay(1000);

    Serial.println("\n=== ĐỌC & GHÉP TIN NHẮN SMS (UCS2, TIẾNG VIỆT) ===\n");

    aiot.begin();

    if (!aiot.init(10000) || !aiot.ensureNetwork(3, 3000)) {
        Serial.println("[-] Lỗi module hoặc mạng!");
        while (true);
    }

    Serial.println("[+] Module sẵn sàng, đang đọc tin nhắn...");

    int count = aiot.getMergedSMSList(smsArr, 20);
    printSMSList(smsArr, count);

    // Nhập số thứ tự muốn xóa qua Serial
    if (count > 0) {
        Serial.println("Nhập số thứ tự tin nhắn muốn xóa (1 - " + String(count) + "): ");
        while (!Serial.available());
        int delIdx = Serial.parseInt() - 1;
        if (delIdx >= 0 && delIdx < count) {
            Serial.printf("Đang xóa các index gốc của tin nhắn số %d: ", delIdx + 1);
            for (int j = 0; j < smsArr[delIdx].originalCount; j++) {
                int origIdx = smsArr[delIdx].originalIndexes[j];
                Serial.printf("%d ", origIdx);
                if (aiot.deleteSMS(origIdx)) {
                    Serial.print("[OK] ");
                } else {
                    Serial.print("[FAIL] ");
                }
            }
            Serial.println("\nĐã xong!");
        } else {
            Serial.println("Số thứ tự không hợp lệ!");
        }

        // Đọc lại và hiển thị danh sách sau khi xóa
        delay(1000); // đợi module cập nhật
        int newCount = aiot.getMergedSMSList(smsArr, 20);
        Serial.println("\n[+] Danh sách tin nhắn sau khi xóa:");
        printSMSList(smsArr, newCount);
    }
}

void loop() {}