#include <Arduino.h>  // Để khai báo kiểu String
#include <WiFi.h>
#include <HTTPClient.h>  // Cho phép khai báo kiểu HTTPClient

// Thông tin WiFi
#define WIFI_SSID "note20"
#define WIFI_PASSWORD "12345678"

#define FIREBASE_HOST "https://hello-world-c7e37-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "AIzaSyBn4pgBRoMRic3gyp67py_vfF2S6pd8Ctw"
int temperature = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(45, OUTPUT);
  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // 1. Gửi dữ liệu lên Firebase (PUT hoặc POST)
    String url = FIREBASE_HOST "/ESP32/Temperature.json?auth=" FIREBASE_AUTH;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // Giá trị cần gửi
    String jsonData = String(temperature);  // Chuyển đổi giá trị thành chuỗi JSON
    temperature++;
    int httpResponseCode = http.PUT(jsonData);  // Dùng PUT để ghi đè giá trị
    if (httpResponseCode > 0) {
      Serial.print("Gửi dữ liệu thành công! Mã phản hồi: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Gửi dữ liệu thất bại! Lỗi: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();

    delay(1000);  // Chờ trước khi gửi yêu cầu tiếp theo

    // 2. Nhận dữ liệu từ Firebase (GET)
    http.begin(FIREBASE_HOST "/LED/digital.json?auth=" FIREBASE_AUTH);
    httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.print("Dữ liệu nhận được: ");
      Serial.println(payload);
    } else {
      Serial.print("Nhận dữ liệu thất bại! Lỗi: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();

    delay(5000);  // Chờ 5 giây trước vòng lặp tiếp theo
  } else {
    Serial.println("WiFi không kết nối!");
    delay(1000);
  }
}
