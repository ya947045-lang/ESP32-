#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

// ======= WiFi + OPENAI =======
#define WIFI_SSID "WE_C5A7E7"
#define WIFI_PASS "k8221770"
#define API_KEY   "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"

const char* MODEL_NAME = "gpt-4o-mini";
const char* OPENAI_URL = "https://api.openai.com/v1/chat/completions";

// ===== OLED =====
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ===== WebServer =====
WiFiClientSecure client;
WebServer server(80);

// ===== Scroll Text =====
String scrollText = "";
int scrollX = 128;

// ===== لوجو AI =====
void showLogo() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso50_tf);
  u8g2.drawStr(45, 45, "AI");
  u8g2.sendBuffer();
  delay(2500);
}

// ===== Scroll OLED =====
void drawScroll() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso30_tf);
  int w = u8g2.getStrWidth(scrollText.c_str());
  u8g2.drawUTF8(scrollX, 50, scrollText.c_str());
  u8g2.sendBuffer();
  scrollX -= 1;
  if (scrollX < -w) scrollX = 128;
}

// ===== إرسال سؤال إلى OpenAI =====
String askOpenAI(String msg) {
  if (WiFi.status() != WL_CONNECTED) return "WiFi not connected";

  HTTPClient http;
  http.begin(client, OPENAI_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + API_KEY);

  // إنشاء JSON body
  StaticJsonDocument<4096> doc;
  doc["model"] = MODEL_NAME;
  JsonArray messages = doc.createNestedArray("messages");
  JsonObject m = messages.createNestedObject();
  m["role"] = "user";
  m["content"] = msg;

  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);
  if (httpCode <= 0) {
    http.end();
    return "Request failed: " + String(httpCode);
  }

  String payload = http.getString();
  Serial.println("Payload: " + payload); // هنا تقدر تشوف الرد من OpenAI
  http.end();

  // تحليل الرد
  DynamicJsonDocument resp(65536);
  auto error = deserializeJson(resp, payload);
  if (error) return "JSON parse error: " + String(error.c_str());

  // لو فيه خطأ من OpenAI
  if (resp.containsKey("error")) {
    return String(resp["error"]["message"].as<const char*>());
  }

  // استخراج الرد
  if (!resp.containsKey("choices") || resp["choices"].size() == 0) return "No reply from OpenAI";
  const char* reply = resp["choices"][0]["message"]["content"];
  if (!reply) return "No reply content";
  
  return String(reply);
}

// ===== صفحة الويب =====
const char htmlPage[] PROGMEM = R"=====(
// ضع هنا نفس صفحة الويب اللي عندك
)=====";

// ===== Web Handlers =====
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleAsk() {
  if (server.hasArg("question")) {
    scrollText = "Processing...";
    scrollX = 128;
    String reply = askOpenAI(server.arg("question"));
    scrollText = reply;
    scrollX = 128;
    server.send(200, "text/plain", reply);
  } else {
    server.send(400, "text/plain", "No question received");
  }
}

void setup() {
  Serial.begin(115200);

  u8g2.begin();
  u8g2.setFont(u8g2_font_6x12_tf);

  // WiFi
  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "Connecting to WI-FI");
  u8g2.sendBuffer();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }

  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "WiFi Connected");
  String ip = "IP: " + WiFi.localIP().toString();
  u8g2.drawStr(0, 35, ip.c_str());
  u8g2.sendBuffer();
  delay(1500);

  showLogo();

  client.setInsecure();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/ask", HTTP_POST, handleAsk);
  server.begin();

  scrollText = "Abo El yosr is ready";
}

void loop() {
  server.handleClient();
  drawScroll();
  delay(10);
}
