const char* OPENAI_URL = "https://api.openai.com/v1/responses";









String askOpenAI(String msg) {
  if (WiFi.status() != WL_CONNECTED) return "WiFi not connected";

  HTTPClient http;
  client.setInsecure();

  http.begin(client, OPENAI_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + API_KEY);

  StaticJsonDocument<2048> doc;
  doc["model"] = MODEL_NAME;
  doc["input"] = msg;

  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);
  if (httpCode <= 0) {
    http.end();
    return "Request failed";
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument resp(16384);
  DeserializationError error = deserializeJson(resp, payload);
  if (error) return "JSON parse error";

  if (!resp["output"] || !resp["output"][0]["content"][0]["text"])
    return "No reply";

  String reply = resp["output"][0]["content"][0]["text"].as<String>();
  return reply;
}










