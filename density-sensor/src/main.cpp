// Archimedes density sensor: a fixed-volume sinker hangs from a load cell
// (HX711). Apparent weight drops by rho*V*g when submerged, so
//
//   rho = rho_water(T) * (raw_air - raw) / (raw_air - raw_water)
//
// where raw_air / raw_water are calibration readings of the same sinker in air
// and in distilled water. Sinker volume and load-cell scale cancel out.
//
// Serial commands: 'a' = store air reading, 'w' = store distilled-water reading
// (type the water temperature if it's not the current probe reading is fine -
// the probe is used automatically).
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

HX711 scale;
OneWire oneWire(DS18B20_PIN);
DallasTemperature tempSensor(&oneWire);
Preferences prefs;

double rawAir = 0, rawWater = 0, calWaterTempC = 20.0;

// Pure-water density in kg/m^3 (Tanaka et al. 2001 polynomial, 0-40 C).
double waterDensity(double t) {
  return 999.842594 + 6.793952e-2 * t - 9.095290e-3 * t * t +
         1.001685e-4 * t * t * t - 1.120083e-6 * t * t * t * t +
         6.536332e-9 * t * t * t * t * t;
}

double readRaw() {
  double sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += scale.read();
  return sum / SAMPLES;
}

double readTempC() {
  tempSensor.requestTemperatures();
  return tempSensor.getTempCByIndex(0);
}

void loadCal() {
  prefs.begin("density", true);
  rawAir = prefs.getDouble("air", 0);
  rawWater = prefs.getDouble("water", 0);
  calWaterTempC = prefs.getDouble("wtemp", 20.0);
  prefs.end();
}

void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c != 'a' && c != 'w') continue;
    double raw = readRaw();
    prefs.begin("density", false);
    if (c == 'a') {
      rawAir = raw;
      prefs.putDouble("air", raw);
    } else {
      rawWater = raw;
      calWaterTempC = readTempC();
      prefs.putDouble("water", raw);
      prefs.putDouble("wtemp", calWaterTempC);
    }
    prefs.end();
    Serial.printf("stored %s raw=%.0f\n", c == 'a' ? "air" : "water", raw);
  }
}

void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) delay(500);
}

void report() {
  double t = readTempC();
  double raw = readRaw();
  if (rawAir == rawWater) {
    Serial.println("not calibrated: press 'a' (sinker in air) then 'w' (in distilled water)");
    return;
  }
  // Water density at calibration temp; the sinker's own expansion is ignored.
  double rho = waterDensity(calWaterTempC) * (rawAir - raw) / (rawAir - rawWater);

  char body[200];
  snprintf(body, sizeof(body),
           "{\"device\":\"%s\",\"density_kg_m3\":%.2f,\"temp_c\":%.2f,\"raw\":%.0f,\"uptime_s\":%lu}",
           DEVICE_ID, rho, t, raw, millis() / 1000);
  Serial.println(body);

  ensureWifi();
  if (WiFi.status() != WL_CONNECTED) { Serial.println("wifi down"); return; }
  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);
  Serial.printf("POST -> %d\n", code);
  http.end();
}

void setup() {
  Serial.begin(115200);
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  tempSensor.begin();
  loadCal();
  WiFi.mode(WIFI_STA);
}

void loop() {
  static uint32_t last = 0;
  handleSerial();
  if (last == 0 || millis() - last >= POST_INTERVAL_S * 1000UL) {
    last = millis();
    report();
  }
  delay(50);
}
