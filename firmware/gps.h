#ifndef GPS_H
#define GPS_H

#include <SoftwareSerial.h>

// Define GPS RX and TX pins (adjust as needed for ESP32)
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

// Initialize SoftwareSerial for GPS
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

// Global variables to hold coordinates
String latitude = "";
String longitude = "";

void initGPS() {
  gpsSerial.begin(9600);
  delay(1000);  // Wait for GPS module to stabilize
}

// Simulate reading GPS data as if it's coming from NMEA sentence
bool readGPS() {
  while (gpsSerial.available()) {
    String line = gpsSerial.readStringUntil('\n');
    if (line.startsWith("$GPGGA")) {
      // Simulated fixed coordinates (replace with real parsing if needed)
      latitude = "28.6139";   // Simulated Latitude
      longitude = "77.2090";  // Simulated Longitude
      return true;
    }
  }
  return false;
}

String getGPSLocation() {
  if (readGPS()) {
    return "Lat: " + latitude + ", Lon: " + longitude;
  } else {
    return "GPS not fixed";
  }
}

#endif
