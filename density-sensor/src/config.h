#pragma once

#define WIFI_SSID     "your-ssid"
#define WIFI_PASS     "your-password"
#define SERVER_URL    "http://192.168.1.50:8000/density"
#define DEVICE_ID     "density-01"

#define POST_INTERVAL_S 60   // how often to report
#define HX711_DOUT_PIN  16
#define HX711_SCK_PIN   17
#define DS18B20_PIN     4    // needs 4.7k pull-up to 3V3
#define SAMPLES         20   // HX711 readings averaged per report
