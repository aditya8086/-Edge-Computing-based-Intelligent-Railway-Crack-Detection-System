#ifndef CAMERA_MODULE_H
#define CAMERA_MODULE_H

#include <Arduino.h>
#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"

// ESP32-CAM pins
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

inline bool initCamera() {
  camera_config_t cfg;
  cfg.ledc_channel   = LEDC_CHANNEL_0;
  cfg.ledc_timer     = LEDC_TIMER_0;
  cfg.pin_d0         = Y2_GPIO_NUM;
  cfg.pin_d1         = Y3_GPIO_NUM;
  cfg.pin_d2         = Y4_GPIO_NUM;
  cfg.pin_d3         = Y5_GPIO_NUM;
  cfg.pin_d4         = Y6_GPIO_NUM;
  cfg.pin_d5         = Y7_GPIO_NUM;
  cfg.pin_d6         = Y8_GPIO_NUM;
  cfg.pin_d7         = Y9_GPIO_NUM;
  cfg.pin_xclk       = XCLK_GPIO_NUM;
  cfg.pin_pclk       = PCLK_GPIO_NUM;
  cfg.pin_vsync      = VSYNC_GPIO_NUM;
  cfg.pin_href       = HREF_GPIO_NUM;
  cfg.pin_sscb_sda   = SIOD_GPIO_NUM;
  cfg.pin_sscb_scl   = SIOC_GPIO_NUM;
  cfg.pin_pwdn       = PWDN_GPIO_NUM;
  cfg.pin_reset      = RESET_GPIO_NUM;
  cfg.xclk_freq_hz   = 20000000;
  cfg.pixel_format   = PIXFORMAT_JPEG;

  if (psramFound()) {
    cfg.frame_size   = FRAMESIZE_VGA;
    cfg.jpeg_quality = 10;
    cfg.fb_count     = 2;
    cfg.fb_location  = CAMERA_FB_IN_PSRAM;
  } else {
    cfg.frame_size   = FRAMESIZE_CIF;
    cfg.jpeg_quality = 12;
    cfg.fb_count     = 1;
    cfg.fb_location  = CAMERA_FB_IN_DRAM;
  }
  cfg.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t err = esp_camera_init(&cfg);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed 0x%x\n", err);
    return false;
  }
  return true;
}

inline String capturePhoto(const char* path = "/crack.jpg") {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "";
  }

  File f = SD_MMC.open(path, FILE_WRITE);
  if (!f) {
    Serial.println("SD open fail");
    esp_camera_fb_return(fb); 
    return "";
  }

  size_t written = f.write(fb->buf, fb->len);
  f.close();
  esp_camera_fb_return(fb);

  if (written != fb->len) {
    Serial.println("Incomplete write");
    return "";
  }

  Serial.println(String("Photo saved → ") + path);
  return String(path);
}

#endif
