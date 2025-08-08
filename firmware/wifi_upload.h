#ifndef WIFI_UPLOAD_H
#define WIFI_UPLOAD_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <SD_MMC.h>
#include "network_config.h"   // brings in WIFI_* and IMG_UPLOAD_URL

inline void ensureWiFi()
{
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print('.');
  }
  Serial.println("\nWi-Fi connected → " + WiFi.localIP().toString());
}

/** Uploads a local JPEG file to the web server.  
 *  Returns the URL returned by the PHP script or empty on failure. */
inline String uploadImage(const String& localPath)
{
  ensureWiFi();

  HTTPClient http;
  http.begin(IMG_UPLOAD_URL);
  http.addHeader("Content-Type", "image/jpeg");

  File pic = SD_MMC.open(localPath, FILE_READ);
  if (!pic) { Serial.println("Failed to open JPEG for upload."); return ""; }

  int code = http.sendRequest("POST", &pic, pic.size());
  pic.close();

  if (code == 200) {
    String url = http.getString();
    Serial.println("Upload OK → " + url);
    http.end();
    return url;
  }
  Serial.printf("Upload failed [%d]\n", code);
  http.end();
  return "";
}

#endif
