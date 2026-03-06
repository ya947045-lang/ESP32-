#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>

const char* ssid = "WE_C5A7E7";
const char* password = "k8221770
";
const char* apiKey = "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";

WebServer server(80);

// ===== بنات الموتور =====
#define MOTOR_A_IA 25
#define MOTOR_A_IB 26
#define MOTOR_B_IA 27      
#define MOTOR_B_IB 14

// ===== System Prompt =====
String systemPrompt = "اسمك هو سبارك، هذا اسمك الوحيد ولا تقبل أي اسم آخر. إذا سألك أحد عن اسمك قل دائماً: اسمي سبارك. إذا سألك أحد من صنعك أو من برمجك أو من طورك قل دائماً: صنعني المهندس ياسر احمد. اسم المستخدم الذي تتحدث معه هو ياسر احمد. ناده دائماً باسمه ياسر. تحدث معه بالعربي دائماً. كن مختصراً ومفيداً. إذا طلب منك ياسر التحرك أو أعطاك أمر حركة مثل امشي أو روح أو تعال أو اتجه، رد برسالتين: الأولى كلمة واحدة فقط من هذه الكلمات FORWARD أو BACKWARD أو RIGHT أو LEFT أو STOP، والثانية رد طبيعي بالعربي يؤكد تنفيذ الأمر. إذا طلب منك ياسر تشغيل مود التحكم اليدوي أو التحكم بالأزرار أو ما شابه ذلك، رد بالنص التالي حرفياً: MANUAL_MODE_ON ثم اكتب رسالة بالعربي تخبره أن مود التحكم اليدوي تم تفعيله.";

// ===== المحادثة =====
const int MAX_MESSAGES = 10;
String roles[MAX_MESSAGES];
String contents[MAX_MESSAGES];
int messageCount = 0;

// ===== وظائف الموتور =====
void moveForward()  { digitalWrite(MOTOR_A_IA, HIGH); digitalWrite(MOTOR_A_IB, LOW);  digitalWrite(MOTOR_B_IA, HIGH); digitalWrite(MOTOR_B_IB, LOW); }
void moveBackward() { digitalWrite(MOTOR_A_IA, LOW);  digitalWrite(MOTOR_A_IB, HIGH); digitalWrite(MOTOR_B_IA, LOW);  digitalWrite(MOTOR_B_IB, HIGH); }
void turnRight()    { digitalWrite(MOTOR_A_IA, HIGH); digitalWrite(MOTOR_A_IB, LOW);  digitalWrite(MOTOR_B_IA, LOW);  digitalWrite(MOTOR_B_IB, HIGH); }
void turnLeft()     { digitalWrite(MOTOR_A_IA, LOW);  digitalWrite(MOTOR_A_IB, HIGH); digitalWrite(MOTOR_B_IA, HIGH); digitalWrite(MOTOR_B_IB, LOW); }
void stopMotors()   { digitalWrite(MOTOR_A_IA, LOW);  digitalWrite(MOTOR_A_IB, LOW);  digitalWrite(MOTOR_B_IA, LOW);  digitalWrite(MOTOR_B_IB, LOW); }

void executeCommand(String response) {
  if (response.indexOf("FORWARD") >= 0)       { moveForward();  delay(1000); stopMotors(); }
  else if (response.indexOf("BACKWARD") >= 0) { moveBackward(); delay(1000); stopMotors(); }
  else if (response.indexOf("RIGHT") >= 0)    { turnRight();    delay(500);  stopMotors(); }
  else if (response.indexOf("LEFT") >= 0)     { turnLeft();     delay(500);  stopMotors(); }
  else if (response.indexOf("STOP") >= 0)     { stopM
// ===== المحادثة =====
void addMessage(String role, String content) {
  content.replace("\"", "'");
  content.replace("\n", " ");
  if (messageCount < MAX_MESSAGES) {
    roles[messageCount] = role;
    contents[messageCount] = content;
    messageCount++;
  } else {
    for (int i = 0; i < MAX_MESSAGES - 1; i++) {
      roles[i] = roles[i + 1];
      contents[i] = contents[i + 1];
    }
    roles[MAX_MESSAGES - 1] = role;
    contents[MAX_MESSAGES - 1] = content;
  }
}

String buildMessages() {
  String messages = "";
  messages += "{\"role\":\"system\",\"content\":\"" + systemPrompt + "\"}";
  for (int i = 0; i < messageCount; i++) {
    messages += ",{\"role\":\"" + roles[i] + "\",\"content\":\"" + contents[i] + "\"}";
  }
  return messages;
}

String askGroq(String question) {
  addMessage("user", question);
  HTTPClient http;
  http.begin("https://api.groq.com/openai/v1/chat/completions");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(apiKey));
  String body = "{\"model\":\"llama-3.1-8b-instant\",\"messages\":[" + buildMessages() + "]}";
  int httpCode = http.POST(body);
  String response = "";
  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      response = doc["choices"][0]["message"]["content"].as<String>();
      addMessage("assistant", response);
      if (response.indexOf("MANUAL_MODE_ON") < 0) {
        executeCommand(response);
      }
    }
  }
  http.end();
  return response;
}

// ===== صفحة الشات =====
const char htmlChat[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>سبارك</title>
<style>
  * { margin: 0; padding: 0; box-sizing: border-box; }
  body { font-family: 'Segoe UI', sans-serif; background: #0f0f1a; color: #fff; height: 100vh; display: flex; flex-direction: column; }
  .header { background: #1a1a2e; padding: 15px; text-align: center; font-size: 22px; font-weight: bold; color: #00d4ff; border-bottom: 2px solid #00d4ff33; }
  .chat-box { flex: 1; overflow-y: auto; padding: 20px; display: flex; flex-direction: column; gap: 12px; }
  .msg { max-width: 75%; padding: 12px 16px; border-radius: 18px; font-size: 15px; line-height: 1.5; animation: fadeIn 0.3s ease; }
  @keyframes fadeIn { from { opacity:0; transform:translateY(10px); } to { opacity:1; transform:translateY(0); } }
  .user-msg { background: #00d4ff22; border: 1px solid #00d4ff55; align-self: flex-end; border-bottom-right-radius: 4px; }
  .ai-msg { background: #1a1a2e; border: 1px solid #ffffff22; align-self: flex-start; border-bottom-left-radius: 4px; }
  .ai-msg .name { color: #00d4ff; font-size: 12px; margin-bottom: 5px; font-weight: bold; }
  .manual-btn { background: #00d4ff; color: #000; border: none; padding: 10px 20px; border-radius: 20px; cursor: pointer; font-size: 14px; font-weight: bold; margin-top: 8px; }
  .input-area { background: #1a1a2e; padding: 15px; border-top: 1px solid #ffffff11; display: flex; gap: 10px; align-items: center; }
  input[type=text] { flex: 1; background: #0f0f1a; border: 1px solid #00d4ff44; color: #fff; padding: 12px 16px; border-radius: 25px; font-size: 15px; outline: none; }
  input[type=text]:focus { border-color: #00d4ff; }
  .btn { width: 48px; height: 48px; border-radius: 50%; border: none; cursor: pointer; font-size: 20px; display: flex; align-items: center; justify-content: center; transition: all 0.2s; }
  .send-btn { background: #00d4ff; color: #000; }
  .mic-btn { background: #1a1a2e; border: 2px solid #00d4ff44; color: #00d4ff; }
  .mic-btn.recording { background: #ff4444; border-color: #ff4444; color: #fff; animation: pulse 1s infinite; }
  @keyframes pulse { 0%,100%{transform:scale(1);} 50%{transform:scale(1.1);} }
  .thinking { color: #00d4ff88; font-size: 13px; padding: 5px 16px; }
</style>
</head>
<body>
<div class="header">⚡ سبارك</div>
<div class="chat-box" id="chatBox">
  <div class="msg ai-msg">
    <div class="name">سبارك</div>
    أهلاً ياسر! أنا سبارك، كيف أقدر أساعدك؟ 😊
  </div>
</div>
<div class="input-area">
  <button class="btn mic-btn" id="micBtn" onclick="toggleMic()">🎤</button>
  <input type="text" id="msgInput" placeholder="اكتب رسالة..." onkeypress="if(event.key==='Enter') sendMsg()">
  <button class="btn send-btn" onclick="sendMsg()">➤</button>
</div>
<script>
  let recognition = null, isRecording = false;
  if ('webkitSpeechRecognition' in window || 'SpeechRecognition' in window) {
    recognition = new (window.SpeechRecognition || window.webkitSpeechRecognition)();
    recognition.continuous = false; recognition.interimResults = false; recognition.lang = 'ar-EG';
    recognition.onresult = e => { document.getElementById('msgInput').value = e.results[0][0].transcript; stopRecording(); sendMsg(); };
    recognition.onerror = () => stopRecording();
    recognition.onend = () => stopRecording();
  }
  function toggleMic() {
    if (!recognition) { alert('المتصفح مش بيدعم التعرف على الصوت!'); return; }
    if (isRecording) { recognition.stop(); }
    else { isRecording = true; document.getElementById('micBtn').classList.add('recording'); document.getElementById('micBtn').innerHTML = '⏹'; recognition.start(); }
  }
  function stopRecording() { isRecording = false; document.getElementById('micBtn').classList.remove('recording'); document.getElementById('micBtn').innerHTML = '🎤'; }
  function addMsg(text, isUser, showManualBtn=false) {
    const box = document.getElementById('chatBox');
    const div = document.createElement('div');
    div.className = 'msg ' + (isUser ? 'user-msg' : 'ai-msg');
    const cleanText = text.replace('MANUAL_MODE_ON', '').trim();
    if (!isUser) {
      div.innerHTML = '<div class="name">سبارك</div>' + cleanText;
      if (showManualBtn) {
        const btn = document.createElement('button');
        btn.className = 'manual-btn';
        btn.textContent = '🕹️ فتح لوحة التحكم';
        btn.onclick = () => window.location.href = '/manual';
        div.appendChild(btn);
      }
    } else { div.textContent = text; }
    box.appendChild(div);
    box.scrollTop = box.scrollHeight;
  }
  function speak(text) {
    const clean = text.replace('MANUAL_MODE_ON', '').trim();
    const utterance = new SpeechSynthesisUtterance(clean);
    utterance.lang = /[\u0600-\u06FF]/.test(clean) ? 'ar-EG' : 'en-US';
    window.speechSynthesis.speak(utterance);
  }
  async function sendMsg() {
    const input = document.getElementById('msgInput');
    const text = input.value.trim();
    if (!text) return;
    input.value = '';
    addMsg(text, true);
    const thinking = document.createElement('div');
    thinking.className = 'thinking'; thinking.textContent = 'سبارك بيفكر...';
    document.getElementById('chatBox').appendChild(thinking);
    try {
      const res = await fetch('/chat', { method: 'POST', headers: {'Content-Type':'application/x-www-form-urlencoded'}, body: 'msg=' + encodeURIComponent(text) });
      const data = await res.text();
      thinking.remove();
      const isManual = data.indexOf('MANUAL_MODE_ON') >= 0;
      addMsg(data, false, isManual);
      speak(data);
    } catch(e) { thinking.remove(); addMsg('حصل خطأ في الاتصال!', false); }
  }
</script>
</body>
</html>
)rawliteral";

// ===== صفحة التحكم اليدوي =====
const char htmlManual[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>تحكم يدوي - سبارك</title>
<style>
  * { margin: 0; padding: 0; box-sizing: border-box; }
  body { font-family: 'Segoe UI', sans-serif; background: #0f0f1a; color: #fff; height: 100vh; display: flex; flex-direction: column; align-items: center; justify-content: center; gap: 20px; }
  .header { font-size: 22px; font-weight: bold; color: #00d4ff; }
  .grid { display: grid; grid-template-columns: repeat(3, 90px); grid-template-rows: repeat(3, 90px); gap: 10px; }
  .ctrl-btn {
    background: #1a1a2e; border: 2px solid #00d4ff44; color: #00d4ff;
    border-radius: 16px; font-size: 30px; cursor: pointer;
    display: flex; align-items: center; justify-content: center;
    transition: all 0.1s; user-select: none; -webkit-user-select: none;
  }
  .ctrl-btn:active, .ctrl-btn.pressed { background: #00d4ff; color: #000; transform: scale(0.95); }
  .empty { background: transparent; border: none; }
  .back-btn { background: #1a1a2e; border: 1px solid #00d4ff44; color: #00d4ff; padding: 10px 25px; border-radius: 20px; cursor: pointer; font-size: 14px; }
  .status { color: #00d4ff88; font-size: 13px; }
</style>
</head>
<body>
<div class="header">🕹️ تحكم يدوي - سبارك</div>
<div class="status" id="status">جاهز للتحكم</div>
<div class="grid">
  <div class="empty"></div>
  <button class="ctrl-btn" id="btn-forward">⬆️</button>
  <div class="empty"></div>
  <button class="ctrl-btn" id="btn-left">⬅️</button>
  <button class="ctrl-btn" id="btn-stop">⏹</button>
  <button class="ctrl-btn" id="btn-right">➡️</button>
  <div class="empty"></div>
  <button class="ctrl-btn" id="btn-backward">⬇️</button>
  <div class="empty"></div>
</div>
<button class="back-btn" onclick="window.location.href='/'">🔙 رجوع للشات</button>
<script>
  const commands = {
    'btn-forward': 'FORWARD',
    'btn-backward': 'BACKWARD',
    'btn-left': 'LEFT',
    'btn-right': 'RIGHT',
    'btn-stop': 'STOP'
  };

  const statusEl = document.getElementById('status');
  const labels = { FORWARD:'للأمام', BACKWARD:'للخلف', LEFT:'يسار', RIGHT:'يمين', STOP:'وقف' };

  async function sendCmd(cmd) {
    try {
      await fetch('/move', { method: 'POST', headers: {'Content-Type':'application/x-www-form-urlencoded'}, body: 'cmd=' + cmd });
      statusEl.textContent = '▶ ' + labels[cmd];
    } catch(e) { statusEl.textContent = 'خطأ في الاتصال!'; }
  }

  Object.keys(commands).forEach(id => {
    const btn = document.getElementById(id);
    const cmd = commands[id];

    // للموبايل
    btn.addEventListener('touchstart', e => { e.preventDefault(); btn.classList.add('pressed'); sendCmd(cmd); });
    btn.addEventListener('touchend',   e => { e.preventDefault(); btn.classList.remove('pressed'); sendCmd('STOP'); });

    // للكمبيوتر
    btn.addEventListener('mousedown', () => { btn.classList.add('pressed'); sendCmd(cmd); });
    btn.addEventListener('mouseup',   () => { btn.classList.remove('pressed'); sendCmd('STOP'); });
    btn.addEventListener('mouseleave',() => { btn.classList.remove('pressed'); sendCmd('STOP'); });
  });
</script>
</body>
</html>
)rawliteral";

void handleRoot()   { server.send(200, "text/html; charset=utf-8", htmlChat); }
void handleManual() { server.send(200, "text/html; charset=utf-8", htmlManual); }

void handleChat() {
  if (server.method() == HTTP_POST) {
    String msg = server.arg("msg");
    String response = askGroq(msg);
    server.send(200, "text/plain; charset=utf-8", response);
  }
}

void handleMove() {
  if (server.method() == HTTP_POST) {
    String cmd = server.arg("cmd");
    if      (cmd == "FORWARD")  moveForward();
    else if (cmd == "BACKWARD") moveBackward();
    else if (cmd == "RIGHT")    turnRight();
    else if (cmd == "LEFT")     turnLeft();
    else if (cmd == "STOP")     stopMotors();
    server.send(200, "text/plain", "OK");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_A_IA, OUTPUT); pinMode(MOTOR_A_IB, OUTPUT);
  pinMode(MOTOR_B_IA, OUTPUT); pinMode(MOTOR_B_IB, OUTPUT);
  stopMotors();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("Connected!");
  Serial.println("افتح المتصفح على: http://" + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/chat", handleChat);
  server.on("/manual", handleManual);
  server.on("/move", handleMove);
  server.begin();
}

void loop() {
  server.handleClient();
}
