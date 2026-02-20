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
    return "Request failed";
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument resp(65536);
  if (deserializeJson(resp, payload)) return "JSON parse error";

  const char* reply = resp["choices"][0]["message"]["content"];
  if (!reply) return "No reply";

  return String(reply);
}

// ===== صفحة الويب =====
const char htmlPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>ESP32 AI Chat</title>
<style>
body{
  font-family: 'Helvetica Neue', Arial, sans-serif;
  background:#ece5dd;
  margin:0;
  padding:0;
}

/* ===== Chat container ===== */
.chat-container{
  width:100%;
  max-width:400px;
  height:90vh;
  background:#fff;
  margin:20px auto;
  border-radius:15px;
  display:flex;
  flex-direction:column;
  box-shadow:0 4px 12px rgba(0,0,0,0.2);
  overflow:hidden;
}

/* ===== Header ===== */
.header{
  background:#075E54;
  color:white;
  padding:15px;
  text-align:center;
  font-size:20px;
  font-weight:bold;
}

/* ===== Messages ===== */
.messages{
  flex:1;
  padding:10px;
  overflow-y:auto;
  display:flex;
  flex-direction:column;
  gap:10px;
  background:#ece5dd;
}

/* User bubble */
.msg-user{
  align-self:flex-end;
  background:#dcf8c6;
  padding:10px 14px;
  border-radius:18px 18px 0 18px;
  max-width:80%;
  word-wrap: break-word;
  box-shadow:0 1px 1px rgba(0,0,0,0.1);
}

/* AI bubble */
.msg-ai{
  align-self:flex-start;
  background:#fff;
  padding:10px 14px;
  border-radius:18px 18px 18px 0;
  max-width:80%;
  word-wrap: break-word;
  box-shadow:0 1px 1px rgba(0,0,0,0.1);
}

/* ===== Input area ===== */
.input-area{
  padding:10px;
  border-top:1px solid #ccc;
  display:flex;
  gap:5px;
  background:#f0f0f0;
}

input{
  flex:1;
  padding:10px;
  border-radius:20px;
  border:1px solid #bbb;
}

button{
  padding:10px 15px;
  border-radius:20px;
  border:none;
  background:#075E54;
  color:white;
  font-weight:bold;
  cursor:pointer;
}

/* ===== Continuous Assistant Button ===== */
#assistantBtn{
  background:#128C7E;
  width:95%;
  margin:5px auto;
  padding:12px;
  border-radius:25px;
  color:white;
  font-size:16px;
  border:none;
  cursor:pointer;
}

/* ===== Sound waves ===== */
#wavesContainer{
  height:50px;
  display:flex;
  justify-content:center;
  align-items:flex-end;
  gap:6px;
  opacity:0;
  transition:0.3s;
}

.wave{
  width:6px;
  height:10px;
  background:#128C7E;
  border-radius:5px;
  box-shadow:0 0 10px #128C7E;
  transition:0.1s;
}
</style>
</head>
<body>

<div class="chat-container">

  <div class="header">Ropote  AI Chat</div>

  <div class="messages" id="messages"></div>

  <select id="voiceSelect">
    <option value="male"> Male Voice</option>
    <option value="female"> Female Voice</option>
  </select>

  <button id="assistantBtn" onclick="toggleAssistant()">Start Continuous Assistant </button>

  <div id="wavesContainer">
    <div class="wave" id="w1"></div>
    <div class="wave" id="w2"></div>
    <div class="wave" id="w3"></div>
    <div class="wave" id="w4"></div>
    <div class="wave" id="w5"></div>
  </div>

  <div class="input-area">
    <input id="question" placeholder="Type your message...">
    <button onclick="sendQuestion()">Send</button>
  </div>

</div>

<script>

/* ===== Messages ===== */
const box = document.getElementById("messages");

function addMessage(text, sender){
  let div = document.createElement("div");
  div.className = sender === "user" ? "msg-user" : "msg-ai";
  div.textContent = text;
  box.appendChild(div);
  box.scrollTop = box.scrollHeight;
}

/* ===== Manual Send ===== */
function sendQuestion(){
  let q=document.getElementById('question').value;
  if(q.length==0) return;

  addMessage(q, "user");
  document.getElementById('question').value="";

  sendToESP(q);
}

/* ===== Send to ESP32 ===== */
function sendToESP(text){
  var xhr=new XMLHttpRequest();
  xhr.open("POST","/ask",true);
  xhr.setRequestHeader("Content-Type","application/x-www-form-urlencoded");

  xhr.onreadystatechange=function(){
    if(xhr.readyState==4 && xhr.status==200){
      var reply = xhr.responseText;
      addMessage(reply, "ai");
      speakAuto(reply);
    }
  };

  xhr.send("question="+encodeURIComponent(text));
}

/* ===== Text-to-Speech ===== */
let speaking = false;

function speakAuto(text){
  let voiceType = document.getElementById("voiceSelect").value;
  let msg = new SpeechSynthesisUtterance(text);
  msg.lang = "en";
  msg.rate = 1;

  let voices = speechSynthesis.getVoices();
  let selectedVoice = voices.find(v => 
      (voiceType === "male"  && v.name.toLowerCase().includes("male")) ||
      (voiceType === "female" && v.name.toLowerCase().includes("female"))
  );
  if(selectedVoice) msg.voice = selectedVoice;

  msg.onstart = () => {
    speaking = true;
    document.getElementById("wavesContainer").style.opacity = 1;
    animateWaves();
  };

  msg.onend = () => {
    speaking = false;
    document.getElementById("wavesContainer").style.opacity = 0;
    if(assistantActive) startListening();
  };

  speechSynthesis.speak(msg);
}

/* ===== Animate Sound Waves ===== */
function animateWaves(){
  if(!speaking) return;
  let strength = Math.random();
  for(let i=1;i<=5;i++){
    let wave = document.getElementById("w"+i);
    let height = 10 + Math.random() * 40 * strength;
    wave.style.height = height + "px";
    let color = `hsl(${160 + strength*160}, 80%, 40%)`;
    wave.style.background = color;
    wave.style.boxShadow = `0 0 15px ${color}`;
  }
  requestAnimationFrame(animateWaves);
}

/* ===== Continuous Assistant ===== */
let assistantActive = false;
let recognition;

function toggleAssistant(){
  if(!assistantActive){
    assistantActive = true;
    document.getElementById("assistantBtn").innerHTML = "Stop Continuous Assistant ";
    startListening();
  } else {
    assistantActive = false;
    document.getElementById("assistantBtn").innerHTML = "Start Continuous Assistant ";
    if(recognition) recognition.stop();
  }
}

function startListening(){
  if(!assistantActive) return;

  window.SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;

  recognition = new SpeechRecognition();
  recognition.lang = "en-US";
  recognition.continuous = true;
  recognition.interimResults = false;

  recognition.onresult = (e)=>{
    let text = e.results[e.results.length-1][0].transcript;
    addMessage(text, "user");
    sendToESP(text);
  };

  recognition.onerror = ()=>{
    if(assistantActive) setTimeout(startListening, 300);
  };

  recognition.onend = ()=>{
    if(assistantActive) startListening();
  };

  recognition.start();
}

/* ===== Auto-enable SpeechSynthesis ===== */
window.onload = () => {
  let s = new SpeechSynthesisUtterance("");
  speechSynthesis.speak(s);
};

</script>

</body>
</html>

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

  // عرض IP
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
