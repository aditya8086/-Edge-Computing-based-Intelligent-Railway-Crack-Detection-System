#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <Arduino.h>
#include <WiFi.h>

constexpr const char* WIFI_SSID     = "____";
constexpr const char* WIFI_PASSWORD = "____";
constexpr const char* IMG_UPLOAD_URL = "_______________";

inline void ensureWiFi(uint32_t timeout_ms = 20000) {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeout_ms) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nWi-Fi OK  IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connect FAILED");
  }
}

inline void connectWiFi(uint32_t timeout_ms = 20000) {
  ensureWiFi(timeout_ms);
}

#endif // NETWORK_CONFIG_H
