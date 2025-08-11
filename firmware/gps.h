#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

#ifndef GPS_RX_PIN
#define GPS_RX_PIN 33
#endif

static HardwareSerial gpsSerial(1);
static TinyGPSPlus gps;

inline void initGPS() {
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, -1);
}

inline String getGPSLocation(uint16_t read_ms = 800) {
  unsigned long t0 = millis();
  while (millis() - t0 < read_ms) {
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
  }

  if (gps.location.isValid() && gps.location.age() < 5000) {
    char buf[24];
    String s = "Lat:";
    dtostrf(gps.location.lat(), 0, 6, buf);
    s += buf;
    s += ", Lon:";
    dtostrf(gps.location.lng(), 0, 6, buf);
    s += buf;
    return s;
  }

  return "GPS: NO FIX";
}

inline bool gpsHasFix(uint32_t max_age_ms = 5000) {
  return gps.location.isValid() && gps.location.age() < max_age_ms;
}

inline bool waitForGPSFix(uint32_t timeout_ms = 30000) {
  unsigned long t0 = millis();
  while (millis() - t0 < timeout_ms) {
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
    if (gpsHasFix()) return true;
    delay(20);
  }
  return false;
}

#endif // GPS_H
