#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>
#include <HardwareSerial.h>

#include "model_data.h"        
#include "network_config.h"    
#include "wifi_upload.h"       
#include "gps.h"               
#include "ultrasonic.h"       
#include "camera_module.h"     

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema_generated.h"
#include "tensorflow/lite/version.h"

constexpr int kTensorArenaSize = 10 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

tflite::MicroErrorReporter micro_error_reporter;
tflite::AllOpsResolver     resolver;
const tflite::Model*       model       = tflite::GetModel(crack_model_tflite);
tflite::MicroInterpreter*  interpreter = nullptr;
TfLiteTensor*              input       = nullptr;
TfLiteTensor*              output      = nullptr;

#define GSM_TX 17   
#define GSM_RX 16   
HardwareSerial sim800(2);  

// Set your alert number here
#define ALERT_PHONE "+91__________"

bool sim800WaitFor(const char* token, uint32_t timeout_ms = 3000) {
  uint32_t t0 = millis();
  String buf;
  while (millis() - t0 < timeout_ms) {
    while (sim800.available()) {
      char c = (char) sim800.read();
      buf += c;
      if (buf.indexOf(token) >= 0) return true;
    }
    delay(5);
  }
  return false;
}

void sim800Init() {
  sim800.println("AT");
  sim800WaitFor("OK", 2000);
  sim800.println("ATE0");       
  sim800WaitFor("OK", 2000);
  sim800.println("AT+CMGF=1");  
  sim800WaitFor("OK", 2000);
}

void sendSMS(const String& txt) {
  sim800.print("AT+CMGS=\"");
  sim800.print(ALERT_PHONE);
  sim800.println("\"");
  if (!sim800WaitFor(">", 3000)) {
    Serial.println("SIM800: no '>' prompt");
    return;
  }
  sim800.print(txt);
  delay(100);
  sim800.write(26); 
  if (sim800WaitFor("OK", 10000)) {
    Serial.println("SMS sent");
  } else {
    Serial.println("SMS send failed");
  }
}

String captureAndUploadPhoto() {
  connectWiFi();                               
  String local = capturePhoto("/crack.jpg");   
  if (local == "") {
    Serial.println("Capture failed");
    return "";
  }
  String url = uploadImage(local);          
  if (url.length() == 0) {
    Serial.println("Upload failed");
  }
  return url;
}

static uint32_t lastAlertMs = 0;
const  uint32_t ALERT_COOLDOWN_MS = 60000;  // 60s

void setup() {
  Serial.begin(115200);
  delay(1200);
  Serial.println("\nBooting...");

  connectWiFi();

  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD_MMC mount failed");
  } else {
    Serial.println("SD_MMC mounted");
  }

  if (!initCamera()) {
    Serial.println("Camera init failed");
  } else {
    Serial.println("Camera ready");
  }

  initUltrasonic();

  initGPS();                                       
  sim800.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);  
  sim800Init();

  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("Model schema mismatch: %d != %d\n",
                  model->version(), TFLITE_SCHEMA_VERSION);
    while (true) delay(10);
  }

  interpreter = new tflite::MicroInterpreter(
      model, resolver, tensor_arena, kTensorArenaSize, &micro_error_reporter);

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed");
    while (true) delay(10);
  }

  input  = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("System ready");
}

void loop() {
  float depth_cm = getSimulatedDepth();
  if (depth_cm < 0) {
    Serial.println("Ultrasonic timeout / out of range");
    delay(500);
    return;
  }
  Serial.printf("Depth: %.2f cm\n", depth_cm);

  if (input->type != kTfLiteFloat32) {
    Serial.println("Unexpected input tensor type");
    delay(2000);
    return;
  }
  input->data.f[0] = depth_cm;

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Inference error");
    delay(2000);
    return;
  }

  float p_crit = output->data.f[0];  // index 0 = Critical
  float p_mod  = output->data.f[1];  // index 1 = Moderate
  float p_non  = output->data.f[2];  // index 2 = Non-Critical

  Serial.printf("Pred: Non=%.2f  Mod=%.2f  Crit=%.2f\n", p_non, p_mod, p_crit);

  const float THRESH_CRIT = 0.80f;
  if (p_crit > THRESH_CRIT && p_crit > p_mod && p_crit > p_non) {
    if (millis() - lastAlertMs >= ALERT_COOLDOWN_MS) {

      if (!gpsHasFix()) {
        Serial.println("Waiting for GPS fix...");
        if (!waitForGPSFix(30000)) {
          Serial.println("GPS fix timeout; skipping alert this cycle");
          return;
        }
      }

      String gps  = getGPSLocation(200);    
      String url  = captureAndUploadPhoto(); 

      String sms  = "CRITICAL crack!\n" + gps + "\n";
      sms += (url.length() ? "Img: " + url : "Img: upload failed");

      sendSMS(sms);
      lastAlertMs = millis();

    } else {
      Serial.println("Alert suppressed (cooldown)");
    }
  }

  delay(5000);
}
