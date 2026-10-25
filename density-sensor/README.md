# ESP32 submerged density sensor

Archimedes method: a fixed-volume sinker hangs from a load cell and is fully
submerged. Density comes from how much apparent weight it loses, corrected by a
DS18B20 temperature reading. The ESP32 POSTs JSON to your server every
`POST_INTERVAL_S` seconds.

## Parts
- ESP32 dev board
- HX711 board + 100-500 g load cell (pick range just above sinker weight)
- DS18B20 waterproof probe + 4.7 k pull-up (DATA to 3V3)
- Sinker: glass or stainless, 20-50 cm^3, no trapped air, hung by thin fishing
  line (thin line = less surface-tension error). Keep it clear of walls/bottom.

## Wiring
| Part | ESP32 |
|---|---|
| HX711 DT / SCK | GPIO16 / GPIO17 |
| HX711 VCC / GND | 3V3 / GND |
| DS18B20 DATA | GPIO4 |

Mount the load cell above the liquid; only the sinker and line get wet.

## Calibration (once, stored in flash)
1. Flash, open serial monitor (115200).
2. Sinker hanging in air, still: send `a`.
3. Lower it into distilled water, wait for temperature to settle: send `w`.

Reported density is relative to that water reading, so no volume or scale
factor is needed. Re-do it if the sinker, line or mounting changes.

## Server
`python3 server/receiver.py` logs to `readings.csv`. Set `SERVER_URL` in
`src/config.h` to `http://<host>:8000/density`.

Payload: `{"device","density_kg_m3","temp_c","raw","uptime_s"}`.

Expect ~0.1-0.5 % accuracy once temperature-stable; bubbles on the sinker and
load-cell drift are the main errors. Untested on hardware.
