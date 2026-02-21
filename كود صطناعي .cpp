#include <WiFi.h>
#include <HTTPClient.h>

#define WIFI_SSID "WE_C5A7E7"
#define WIFI_PASS "k8221770"

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // اختبر الإنترنت
  testInternet();
}

void loop() {
  // ممكن تحط هنا أي كود ثاني
}

void testInternet() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://example.com");  // موقع بسيط وموثوق
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.println("ESP32 is ONLINE! 🌐");
      Serial.print("HTTP code: ");
      Serial.println(httpCode);
    } else {
      Serial.println("ESP32 is OFFLINE 😓");
      Serial.print("Error code: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
