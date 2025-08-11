#ifndef WIFI_UPLOAD_H
#define WIFI_UPLOAD_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SD_MMC.h>
#include "network_config.h"   

inline String uploadImage(const String& localPath,
                          uint8_t retries = 2,
                          uint16_t timeout_ms = 10000) {
  for (uint8_t attempt = 0; attempt <= retries; ++attempt) {
    ensureWiFi();

    File pic = SD_MMC.open(localPath, FILE_READ);
    if (!pic) {
      Serial.println("uploadImage: failed to open JPEG on SD");
      return "";
    }

    HTTPClient http;
    http.setTimeout(timeout_ms);

    if (!http.begin(IMG_UPLOAD_URL)) {
      Serial.println("uploadImage: http.begin() failed");
      pic.close();
      delay(300);
      continue;
    }

    http.addHeader("Content-Type", "image/jpeg");

    int code = http.sendRequest("POST", &pic, pic.size());
    String body = (code > 0) ? http.getString() : "";

    http.end();
    pic.close();

    if (code == 200 && body.length() && body.startsWith("http")) {
      Serial.println("Upload OK: " + body);
      return body;
    }

    Serial.printf("uploadImage: HTTP %d, resp: %s\n", code, body.c_str());
    delay(500); 
  }
  return "";
}

#endif // WIFI_UPLOAD_H
