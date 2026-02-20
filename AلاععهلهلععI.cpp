#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ======= إعدادات WiFi =======
const char* ssid = "اسم الشبكة بتاعتك";
const char* password = "كلمة السر";

// ======= إعدادات OpenAI =======
const char* openai_api_key = "حط هنا مفتاح OpenAI بتاعك";
const char* openai_endpoint = "https://api.openai.com/v1/chat/completions";

// ======= إعداد ESP32 =======
String userMessage = "";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("Type your message and press enter:");
}

void loop() {
  if (Serial.available()) {
    userMessage = Serial.readStringUntil('\n');
    userMessage.trim();
    
    if(userMessage.length() > 0){
      sendToChatGPT(userMessage);
    }
  }
}

void sendToChatGPT(String prompt){
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin(openai_endpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(openai_api_key));
    
    // JSON للجسم المرسل
    String requestBody = "{\"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \"" + prompt + "\"}]}";
    
    int httpResponseCode = http.POST(requestBody);
    
    if(httpResponseCode > 0){
      String response = http.getString();
      
      // استخراج الرد من JSON
      DynamicJsonDocument doc(4096);
      deserializeJson(doc, response);
      String botReply = doc["choices"][0]["message"]["content"];
      
      Serial.println("\nChatGPT:");
      Serial.println(botReply);
      Serial.println("\nاكتب رسالتك:");
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi Disconnected!");
  }
}
