#define BLYNK_TEMPLATE_ID "TMPL6bmimp5Nx"
#define BLYNK_TEMPLATE_NAME "Automatic Watering"
#define BLYNK_AUTH_TOKEN "qjDyvans3nQv4RzVSRWwDpmgu40pSSL1"

#include <WiFi.h>
#include <WebServer.h>
#include <BlynkSimpleEsp32.h>
#include <ESPmDNS.h>

char ssid[] = "PhamQuangTrung";
char pass[] = "h4ymwyx6";

int soilSen = 35;
int waterSen = 34;
int pumpPin = 25;

int moiThreshold = 2000;
int levThreshold = 2000;

bool manualPumpState = false;
bool autoModeEnabled = true;

BlynkTimer timer;
WebServer server(80);

// ---------------------- SEND TO BLYNK ---------------------

void sendMoisture() {
  float raw = analogRead(soilSen);
  int pct = 100 - (raw / 4095.0 * 100);
  Blynk.virtualWrite(V0, pct);
}

void sendLevel() {
  float raw = analogRead(waterSen);
  int pct = (raw / 4095.0 * 100);
  Blynk.virtualWrite(V1, pct);
}


BLYNK_WRITE(V2) {
  float p = param.asFloat();
  moiThreshold = 4095 - (p / 100.0 * 4095);

  Serial.print("[BLYNK] New Moisture Threshold RAW = ");
  Serial.println(moiThreshold);
}

BLYNK_WRITE(V3) {
  float p = param.asFloat();
  levThreshold = (p / 100.0 * 4095);

  Serial.print("[BLYNK] New Water Threshold RAW = ");
  Serial.println(levThreshold);
}

BLYNK_WRITE(V4) {
  autoModeEnabled = param.asInt();
  Serial.print("[BLYNK] Auto Mode = ");
  Serial.println(autoModeEnabled);
}

BLYNK_WRITE(V5) {
  if (!autoModeEnabled) {
    manualPumpState = param.asInt();
    digitalWrite(pumpPin, manualPumpState);
    Serial.print("[BLYNK] Manual Pump = ");
    Serial.println(manualPumpState);
  } else {
    manualPumpState = false;
    Blynk.virtualWrite(V5, 0);
  }
}

// ---------------------- HTTP API ---------------------

void apiData() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  int soil = analogRead(soilSen);
  int water = analogRead(waterSen);
  int moisPct = 100 - (soil / 4095.0 * 100);
  int waterPct = (water / 4095.0 * 100);

  String json = "{";
  json += "\"mois\":" + String(moisPct) + ",";
  json += "\"water\":" + String(waterPct);
  json += "}";

  server.send(200, "application/json", json);
}

void apiSetMoisture() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (!server.hasArg("val")) 
    return server.send(400, "text/plain", "val missing");

  int pct = server.arg("val").toInt();
  moiThreshold = 4095 - (pct / 100.0 * 4095);

  Blynk.virtualWrite(V2, pct);
  Serial.print("[API] New Moisture Threshold RAW = ");
  Serial.println(moiThreshold);

  server.send(200, "text/plain", "OK");
}

void apiSetWater() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (!server.hasArg("val")) 
    return server.send(400, "text/plain", "val missing");

  int pct = server.arg("val").toInt();
  levThreshold = (pct / 100.0 * 4095);

  Blynk.virtualWrite(V3, pct);
  Serial.print("[API] New Water Threshold RAW = ");
  Serial.println(levThreshold);

  server.send(200, "text/plain", "OK");
}

void apiToggleAuto() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  int s = server.arg("state").toInt();
  autoModeEnabled = s;

  Blynk.virtualWrite(V4, s);

  Serial.print("[API] Auto Mode = ");
  Serial.println(autoModeEnabled);

  server.send(200, "text/plain", "OK");
}

void apiTogglePump() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  int s = server.arg("state").toInt();
  if (!autoModeEnabled) {
    manualPumpState = s;
    digitalWrite(pumpPin, s);
    Blynk.virtualWrite(V5, s);

    Serial.print("[API] Manual Pump = ");
    Serial.println(manualPumpState);
  }
  server.send(200, "text/plain", "OK");
}

void apiState() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  int moisThresPct = 0;
  if (moiThreshold > 0) {
      moisThresPct = 100 - (int)(moiThreshold / 4095.0 * 100);
  }

  int levThresPct = (int)(levThreshold / 4095.0 * 100);

  String json = "{";
  json += "\"auto\":" + String(autoModeEnabled) + ",";
  json += "\"pump\":" + String(manualPumpState) + ","; 
  json += "\"moisThres\":" + String(moisThresPct) + ","; 
  json += "\"levThres\":" + String(levThresPct);
  json += "}";

  server.send(200, "application/json", json);
}

unsigned long lastDebug = 0;

void debugPrint() {
  int soil = analogRead(soilSen);
  int water = analogRead(waterSen);

  int moisPct = 100 - (soil / 4095.0 * 100);
  int waterPct = (water / 4095.0 * 100);

  Serial.print("Soil RAW="); Serial.print(soil);
  Serial.print(" | Moisture%="); Serial.print(moisPct);
  Serial.print(" | MoiThrRAW="); Serial.print(moiThreshold);

  Serial.print(" || Water RAW="); Serial.print(water);
  Serial.print(" | Water%="); Serial.print(waterPct);
  Serial.print(" | WatThrRAW="); Serial.print(levThreshold);

  Serial.print(" || AutoMode="); Serial.print(autoModeEnabled);
  Serial.print(" | ManualPump="); Serial.print(manualPumpState);
  Serial.print(" | PumpPin="); Serial.println(digitalRead(pumpPin));
}

void setup() {
  Serial.begin(115200);

  pinMode(soilSen, INPUT);
  pinMode(waterSen, INPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { delay(200); }

  Serial.print("[SYSTEM] IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("vuonthongminh")) {
    Serial.println("MDNS responder started");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, sendMoisture);
  timer.setInterval(1000L, sendLevel);

  server.on("/state", apiState);
  server.on("/data", apiData);
  server.on("/set/mois", apiSetMoisture);
  server.on("/set/water", apiSetWater);
  server.on("/toggle/auto", apiToggleAuto);
  server.on("/toggle/pump", apiTogglePump);
  server.begin();

  Serial.println("[SYSTEM] Setup completed.");
}

void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();

  int soil = analogRead(soilSen);
  int water = analogRead(waterSen);

  if (autoModeEnabled == 1) {
    bool dry = (soil >= moiThreshold);
    bool enough = (water >= levThreshold);
    digitalWrite(pumpPin, (dry && enough) ? HIGH : LOW);
  } 
  else {
    digitalWrite(pumpPin, manualPumpState ? HIGH : LOW);
  }

  unsigned long now = millis();
  if (now - lastDebug >= 500) {
    lastDebug = now;
    debugPrint();
  }
}
