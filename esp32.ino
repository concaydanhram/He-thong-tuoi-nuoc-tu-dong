#define BLYNK_TEMPLATE_ID "TMPL6bmimp5Nx"
#define BLYNK_TEMPLATE_NAME "Automatic Watering"
#define BLYNK_AUTH_TOKEN "qjDyvans3nQv4RzVSRWwDpmgu40pSSL1"

#include <WiFi.h>
#include <WebServer.h>
#include <BlynkSimpleEsp32.h>
#include <ESPmDNS.h>

#define ADC_MAX 4095

const char* SSID = "PhamQuangTrung";
const char* PASS = "h4ymwyx6";

//PIN
const int PIN_SOIL  = 35;
const int PIN_WATER = 34;
const int PIN_PUMP  = 25;

//STATE
int soilRaw = 0;
int waterRaw = 0;
int moiThreshold = 2000;
int levThreshold = 2000;
bool autoMode = true;
bool pumpManual = false;

//SYSTEM
WebServer server(80);
BlynkTimer timer;

int clamp(int v, int minV, int maxV) {
  if (v < minV) return minV;
  if (v > maxV) return maxV;
  return v;
}

int rawToMoisturePct(int raw) {
  return 100 - (raw * 100 / ADC_MAX);
}

int rawToWaterPct(int raw) {
  return raw * 100 / ADC_MAX;
}

int pctToRawMoisture(int pct) {
  return ADC_MAX - (pct * ADC_MAX / 100);
}

int pctToRawWater(int pct) {
  return pct * ADC_MAX / 100;
}

//SENSOR
void readSensors() {
  soilRaw = analogRead(PIN_SOIL);
  waterRaw = analogRead(PIN_WATER);
}

//CONTROL
void updatePump() {
  if (autoMode) {
    bool dry = (soilRaw >= moiThreshold);
    bool enough = (waterRaw >= levThreshold);
    digitalWrite(PIN_PUMP, (dry && enough));
  } else {
    digitalWrite(PIN_PUMP, pumpManual);
  }
}

//BLYNK
void sendToBlynk() {
  Blynk.virtualWrite(V0, rawToMoisturePct(soilRaw));
  Blynk.virtualWrite(V1, rawToWaterPct(waterRaw));
}

BLYNK_WRITE(V2) {
  int pct = clamp(param.asInt(), 0, 100);
  moiThreshold = pctToRawMoisture(pct);
}

BLYNK_WRITE(V3) {
  int pct = clamp(param.asInt(), 0, 100);
  levThreshold = pctToRawWater(pct);
}

BLYNK_WRITE(V4) {
  autoMode = param.asInt();
}

BLYNK_WRITE(V5) {
  if (!autoMode) {
    pumpManual = param.asInt();
  }
}

//HTTP
void sendJSON(String json) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void apiData() {
  String json = "{";
  json += "\"mois\":" + String(rawToMoisturePct(soilRaw)) + ",";
  json += "\"water\":" + String(rawToWaterPct(waterRaw));
  json += "}";
  sendJSON(json);
}

void apiState() {
  String json = "{";
  json += "\"auto\":" + String(autoMode) + ",";
  json += "\"pump\":" + String(pumpManual) + ",";
  json += "\"moisThres\":" + String(rawToMoisturePct(moiThreshold)) + ",";
  json += "\"levThres\":" + String(rawToWaterPct(levThreshold));
  json += "}";
  sendJSON(json);
}

void apiSetMois() {
  if (!server.hasArg("val")) return server.send(400, "text/plain", "missing");

  int pct = clamp(server.arg("val").toInt(), 0, 100);
  moiThreshold = pctToRawMoisture(pct);

  Blynk.virtualWrite(V2, pct);
  server.send(200, "text/plain", "OK");
}

void apiSetWater() {
  if (!server.hasArg("val")) return server.send(400, "text/plain", "missing");

  int pct = clamp(server.arg("val").toInt(), 0, 100);
  levThreshold = pctToRawWater(pct);

  Blynk.virtualWrite(V3, pct);
  server.send(200, "text/plain", "OK");
}

void apiAuto() {
  autoMode = server.arg("state").toInt();
  Blynk.virtualWrite(V4, autoMode);
  server.send(200, "text/plain", "OK");
}

void apiPump() {
  if (!autoMode) {
    pumpManual = server.arg("state").toInt();
    Blynk.virtualWrite(V5, pumpManual);
  }
  server.send(200, "text/plain", "OK");
}

//DEBUG
void debug() {
  Serial.printf("Soil=%d (%d%%) | Water=%d (%d%%) | Auto=%d | Pump=%d\n",
    soilRaw, rawToMoisturePct(soilRaw),
    waterRaw, rawToWaterPct(waterRaw),
    autoMode, digitalRead(PIN_PUMP)
  );
}


void setup() {
  Serial.begin(115200);

  pinMode(PIN_PUMP, OUTPUT);

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(200);

  Serial.println(WiFi.localIP());

  MDNS.begin("vuonthongminh");

  Blynk.begin(BLYNK_AUTH_TOKEN, SSID, PASS);

  timer.setInterval(1000L, sendToBlynk);

  server.on("/data", apiData);
  server.on("/state", apiState);
  server.on("/set/mois", apiSetMois);
  server.on("/set/water", apiSetWater);
  server.on("/toggle/auto", apiAuto);
  server.on("/toggle/pump", apiPump);
  server.begin();
}

void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();

  readSensors();
  updatePump();

  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    debug();
  }
}
