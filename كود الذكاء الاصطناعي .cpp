const char* OPENAI_URL = "https://api.openai.com/v1/responses";









String askOpenAI(String msg) {
  if (WiFi.status() != WL_CONNECTED)
    return "WiFi not connected";

  HTTPClient http;
  http.setTimeout(20000);
  http.begin("https://api.openai.com/v1/responses");

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + API_KEY);

  StaticJsonDocument<1024> doc;
  doc["model"] = MODEL_NAME;
  doc["input"] = msg;
  doc["max_output_tokens"] = 150;

  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);
  Serial.println("HTTP Code: " + String(httpCode));

  if (httpCode <= 0) {
    http.end();
    return "Request failed";
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument resp(8192);
  if (deserializeJson(resp, payload))
    return "JSON error";

  if (!resp["output"][0]["content"][0]["text"])
    return "No reply";

  return resp["output"][0]["content"][0]["text"].as<String>();
}
