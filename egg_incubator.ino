/*
 
SMART EGG INCUBATOR - ESP8266 Firmware        
       فقاسة بيض ذكية - نظام تحكم متكامل                
 Hardware:
  - ESP8266 (NodeMCU / Wemos D1 Mini)
  - DHT11 (Temperature & Humidity)
  - Relay 1 → Heating Lamp
  - Relay 2 → Cooling Fan
  - Servo Motor → Egg Turner
 
 Web Server on Access Point: http://192.168.4.1
#includ
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

// ─── Pin Definitions ───────────────────────────────────────────
#define DHT_PIN        D4   // GPIO2
#define RELAY_HEAT_PIN D1   // GPIO5  - Active LOW
#define RELAY_FAN_PIN  D2   // GPIO4  - Active LOW
#define SERVO_PIN      D5   // GPIO14

// ─── DHT Sensor ────────────────────────────────────────────────
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ─── Servo ─────────────────────────────────────────────────────
Servo eggServo;
#define SERVO_POS_A   30    // degrees
#define SERVO_POS_B   150   // degrees

// ─── WiFi AP Config ────────────────────────────────────────────
const char* AP_SSID     = "EggIncubator";
const char* AP_PASSWORD = "12345678";
const IPAddress AP_IP(192, 168, 4, 1);

// ─── Web Server ────────────────────────────────────────────────
ESP8266WebServer server(80);

// ─── EEPROM Addresses ──────────────────────────────────────────
#define EEPROM_SIZE       64
#define ADDR_TARGET_TEMP   0   // float (4 bytes)
#define ADDR_TARGET_HUM    4   // float (4 bytes)
#define ADDR_TURN_INTERVAL 8   // uint32 (4 bytes) - hours in minutes
#define ADDR_INCUB_DAYS   12   // uint8 (1 byte)
#define ADDR_DAY_COUNT    13   // uint16 (2 bytes)
#define ADDR_MODE         15   // uint8 (1 byte)

// ─── Incubator State ───────────────────────────────────────────
struct IncubatorState {
  float   currentTemp    = 0.0;
  float   currentHum     = 0.0;
  float   targetTemp     = 37.8;  // °C
  float   targetHum      = 60.0;  // %
  float   tempHysteresis = 0.5;
  float   humHysteresis  = 5.0;
  bool    heaterOn       = false;
  bool    fanOn          = false;
  bool    autoMode       = true;
  bool    manualHeat     = false;
  bool    manualFan      = false;
  uint8_t incubationDays = 21;    // total days (21 for chicken)
  uint16_t dayCount      = 0;     // elapsed days
  uint32_t turnInterval  = 240;   // minutes between turns (4h)
  uint32_t lastTurnTime  = 0;
  bool    servoPosition  = false; // false=A, true=B
  bool    turning        = false;
  unsigned long startMillis = 0;
  float   minTemp = 99.9;
  float   maxTemp = -99.9;
  float   minHum  = 99.9;
  float   maxHum  = -99.9;
  unsigned long lastDHTRead = 0;
  bool    dhtError = false;
} state;

// ─── Timers ────────────────────────────────────────────────────
unsigned long lastSensorRead  = 0;
unsigned long lastDayCheck    = 0;
unsigned long dayStartMillis  = 0;

// ═══════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\n╔═══════════════════════════╗"));
  Serial.println(F("║   Smart Egg Incubator     ║"));
  Serial.println(F("╚═══════════════════════════╝"));

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

  // Pins
  pinMode(RELAY_HEAT_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN,  OUTPUT);
  digitalWrite(RELAY_HEAT_PIN, HIGH);  // OFF (active low)
  digitalWrite(RELAY_FAN_PIN,  HIGH);  // OFF

  // Servo
  eggServo.attach(SERVO_PIN);
  eggServo.write(SERVO_POS_A);
  delay(500);
  eggServo.detach();

  // DHT
  dht.begin();

  // WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print(F("AP IP: "));
  Serial.println(WiFi.softAPIP());

  // Web routes
  setupRoutes();
  server.begin();

  state.startMillis = millis();
  dayStartMillis    = millis();

  Serial.println(F("✓ System Ready → http://192.168.4.1"));
}

// ═══════════════════════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════════════════════
void loop() {
  server.handleClient();
  
  unsigned long now = millis();

  // Read sensor every 3 seconds
  if (now - lastSensorRead >= 3000) {
    lastSensorRead = now;
    readSensors();
    if (state.autoMode) runAutoControl();
  }

  // Day counter (every minute check)
  if (now - lastDayCheck >= 60000) {
    lastDayCheck = now;
    updateDayCount();
  }

  // Egg turner check
  checkTurnSchedule();
}

// ═══════════════════════════════════════════════════════════════
//  SENSOR READING
// ═══════════════════════════════════════════════════════════════
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    state.dhtError = true;
    Serial.println(F("⚠ DHT Read Error"));
    return;
  }

  state.dhtError     = false;
  state.currentTemp  = t;
  state.currentHum   = h;
  state.lastDHTRead  = millis();

  // Track min/max
  if (t < state.minTemp) state.minTemp = t;
  if (t > state.maxTemp) state.maxTemp = t;
  if (h < state.minHum)  state.minHum  = h;
  if (h > state.maxHum)  state.maxHum  = h;
}

// ═══════════════════════════════════════════════════════════════
//  AUTO CONTROL (Hysteresis)
// ═══════════════════════════════════════════════════════════════
void runAutoControl() {
  if (state.dhtError) return;

  // --- Heating Control ---
  float tempLow  = state.targetTemp - state.tempHysteresis;
  float tempHigh = state.targetTemp + state.tempHysteresis;

  if (state.currentTemp < tempLow && !state.heaterOn) {
    setHeater(true);
  } else if (state.currentTemp >= tempHigh && state.heaterOn) {
    setHeater(false);
  }

  // --- Fan / Humidity Control ---
  // Fan helps regulate humidity AND temperature distribution
  float humLow  = state.targetHum - state.humHysteresis;
  float humHigh = state.targetHum + state.humHysteresis;

  if (state.currentHum > humHigh && !state.fanOn) {
    setFan(true);
  } else if (state.currentHum <= humLow && state.fanOn) {
    setFan(false);
  }

  // Safety: if temp too high, force fan on
  if (state.currentTemp > state.targetTemp + 2.0) {
    setFan(true);
  }
}

// ═══════════════════════════════════════════════════════════════
//  RELAY CONTROL
// ═══════════════════════════════════════════════════════════════
void setHeater(bool on) {
  state.heaterOn = on;
  digitalWrite(RELAY_HEAT_PIN, on ? LOW : HIGH);
  Serial.printf("🔥 Heater: %s\n", on ? "ON" : "OFF");
}

void setFan(bool on) {
  state.fanOn = on;
  digitalWrite(RELAY_FAN_PIN, on ? LOW : HIGH);
  Serial.printf("💨 Fan: %s\n", on ? "ON" : "OFF");
}

// ═══════════════════════════════════════════════════════════════
//  EGG TURNER
// ═══════════════════════════════════════════════════════════════
void checkTurnSchedule() {
  if (state.turning) return;

  unsigned long intervalMs = (unsigned long)state.turnInterval * 60UL * 1000UL;
  if (millis() - state.lastTurnTime >= intervalMs) {
    turnEggs();
  }
}

void turnEggs() {
  Serial.println(F("🥚 Turning eggs..."));
  state.turning = true;

  eggServo.attach(SERVO_PIN);
  state.servoPosition = !state.servoPosition;
  eggServo.write(state.servoPosition ? SERVO_POS_B : SERVO_POS_A);
  
  delay(1000);  // Wait for servo to reach position
  eggServo.detach();
  
  state.lastTurnTime = millis();
  state.turning = false;
  Serial.printf("✓ Eggs at position %s\n", state.servoPosition ? "B" : "A");
}

// ═══════════════════════════════════════════════════════════════
//  DAY COUNTER
// ═══════════════════════════════════════════════════════════════
void updateDayCount() {
  unsigned long elapsed = millis() - state.startMillis;
  state.dayCount = (uint16_t)(elapsed / 86400000UL);
}

// ═══════════════════════════════════════════════════════════════
//  EEPROM SAVE / LOAD
// ═══════════════════════════════════════════════════════════════
void saveSettings() {
  EEPROM.put(ADDR_TARGET_TEMP,    state.targetTemp);
  EEPROM.put(ADDR_TARGET_HUM,     state.targetHum);
  EEPROM.put(ADDR_TURN_INTERVAL,  state.turnInterval);
  EEPROM.put(ADDR_INCUB_DAYS,     state.incubationDays);
  EEPROM.put(ADDR_DAY_COUNT,      state.dayCount);
  EEPROM.commit();
}

void loadSettings() {
  float t, h;
  uint32_t ti;
  uint8_t  id;
  uint16_t dc;

  EEPROM.get(ADDR_TARGET_TEMP,    t);
  EEPROM.get(ADDR_TARGET_HUM,     h);
  EEPROM.get(ADDR_TURN_INTERVAL,  ti);
  EEPROM.get(ADDR_INCUB_DAYS,     id);
  EEPROM.get(ADDR_DAY_COUNT,      dc);

  // Validate
  if (t > 30.0 && t < 42.0) state.targetTemp    = t;
  if (h > 20.0 && h < 90.0) state.targetHum     = h;
  if (ti > 30  && ti < 1440) state.turnInterval = ti;
  if (id > 0   && id < 100)  state.incubationDays = id;
}

// ═══════════════════════════════════════════════════════════════
//  WEB SERVER ROUTES
// ═══════════════════════════════════════════════════════════════
void setupRoutes() {
  // CORS headers helper
  server.on("/",        HTTP_GET,  handleRoot);
  server.on("/api/status", HTTP_GET,  handleStatus);
  server.on("/api/control", HTTP_POST, handleControl);
  server.on("/api/settings", HTTP_POST, handleSettings);
  server.on("/api/reset-stats", HTTP_POST, handleResetStats);
  server.on("/api/turn-now", HTTP_POST, handleTurnNow);
  server.onNotFound([]() {
    server.send(404, "application/json", "{\"error\":\"Not found\"}");
  });
}

// ─── CORS + Cache helpers ───────────────────────────────────────
void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ─── GET / ─────────────────────────────────────────────────────
void handleRoot() {
  addCORSHeaders();
  server.sendHeader("Content-Encoding", "identity");
  server.send(200, "text/html; charset=utf-8", getHTML());
}

// ─── GET /api/status ───────────────────────────────────────────
void handleStatus() {
  addCORSHeaders();
  
  StaticJsonDocument<512> doc;
  doc["temp"]         = state.currentTemp;
  doc["hum"]          = state.currentHum;
  doc["targetTemp"]   = state.targetTemp;
  doc["targetHum"]    = state.targetHum;
  doc["heaterOn"]     = state.heaterOn;
  doc["fanOn"]        = state.fanOn;
  doc["autoMode"]     = state.autoMode;
  doc["day"]          = state.dayCount;
  doc["totalDays"]    = state.incubationDays;
  doc["turnInterval"] = state.turnInterval;
  doc["lastTurn"]     = (millis() - state.lastTurnTime) / 60000;  // minutes ago
  doc["nextTurn"]     = state.turnInterval - ((millis() - state.lastTurnTime) / 60000);
  doc["servoPos"]     = state.servoPosition ? "B" : "A";
  doc["minTemp"]      = state.minTemp;
  doc["maxTemp"]      = state.maxTemp;
  doc["minHum"]       = state.minHum;
  doc["maxHum"]       = state.maxHum;
  doc["dhtError"]     = state.dhtError;
  doc["uptime"]       = millis() / 1000;
  doc["freeHeap"]     = ESP.getFreeHeap();

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// ─── POST /api/control ─────────────────────────────────────────
void handleControl() {
  addCORSHeaders();
  
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (doc.containsKey("autoMode")) {
    state.autoMode = doc["autoMode"].as<bool>();
    if (!state.autoMode) {
      // Keep current relay states when switching to manual
    }
  }
  if (doc.containsKey("heater") && !state.autoMode) {
    setHeater(doc["heater"].as<bool>());
  }
  if (doc.containsKey("fan") && !state.autoMode) {
    setFan(doc["fan"].as<bool>());
  }

  server.send(200, "application/json", "{\"ok\":true}");
}

// ─── POST /api/settings ────────────────────────────────────────
void handleSettings() {
  addCORSHeaders();

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (doc.containsKey("targetTemp")) {
    float v = doc["targetTemp"];
    if (v >= 30.0 && v <= 42.0) state.targetTemp = v;
  }
  if (doc.containsKey("targetHum")) {
    float v = doc["targetHum"];
    if (v >= 20.0 && v <= 90.0) state.targetHum = v;
  }
  if (doc.containsKey("turnInterval")) {
    uint32_t v = doc["turnInterval"];
    if (v >= 30 && v <= 1440) state.turnInterval = v;
  }
  if (doc.containsKey("incubationDays")) {
    uint8_t v = doc["incubationDays"];
    if (v > 0 && v < 100) state.incubationDays = v;
  }

  saveSettings();
  server.send(200, "application/json", "{\"ok\":true}");
}

// ─── POST /api/reset-stats ─────────────────────────────────────
void handleResetStats() {
  addCORSHeaders();
  state.minTemp = 99.9; state.maxTemp = -99.9;
  state.minHum  = 99.9; state.maxHum  = -99.9;
  state.startMillis = millis();
  state.dayCount = 0;
  saveSettings();
  server.send(200, "application/json", "{\"ok\":true}");
}

// ─── POST /api/turn-now ────────────────────────────────────────
void handleTurnNow() {
  addCORSHeaders();
  turnEggs();
  server.send(200, "application/json", "{\"ok\":true}");
}

// ═══════════════════════════════════════════════════════════════
//  HTML PAGE (Embedded)
// ═══════════════════════════════════════════════════════════════
String getHTML() {
  // Returned from separate function to keep code clean
  // The full HTML is defined in html_page.h
  return HTML_PAGE;
}
