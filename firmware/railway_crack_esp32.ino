// edge_crack_detection.ino
#include "model_data.h"
#include "gps.h"
#include "ultrasonic.h"
#include "camera_module.h"
#include "network_config.h"

#include <WiFi.h>
#include <SoftwareSerial.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// TensorFlow Lite globals
constexpr int kTensorArenaSize = 10 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
tflite::MicroErrorReporter micro_error_reporter;
tflite::AllOpsResolver resolver;
const tflite::Model* model = ::tflite::GetModel(g_model_data);
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// GSM Module
#define GSM_TX 17
#define GSM_RX 16
SoftwareSerial sim800(GSM_TX, GSM_RX);

void sendSMS(const String& message) {
  sim800.println("AT+CMGF=1");
  delay(500);
  sim800.println("AT+CMGS=\"+91XXXXXXXXXX\""); // Replace with real number
  delay(500);
  sim800.print(message);
  delay(500);
  sim800.write(26);
  Serial.println("SMS sent:\n" + message);
}

void setup() {
  Serial.begin(115200);
  sim800.begin(9600);
  initGPS();
  initCamera();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");

  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  interpreter = new tflite::MicroInterpreter(
    model, resolver, tensor_arena, kTensorArenaSize, &micro_error_reporter);
  interpreter->AllocateTensors();

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("System initialized.");
}

void loop() {
  float test_depth = getSimulatedDepth();
  input->data.f[0] = test_depth;

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Inference failed!");
    return;
  }

  float non_critical = output->data.f[0];
  float moderate     = output->data.f[1];
  float critical     = output->data.f[2];

  Serial.println("Crack Severity Prediction:");
  Serial.print("  Non-Critical: "); Serial.println(non_critical, 4);
  Serial.print("  Moderate:     "); Serial.println(moderate, 4);
  Serial.print("  Critical:     "); Serial.println(critical, 4);

  if (critical > 0.80 && critical > moderate && critical > non_critical) {
    String location = getGPSLocation();
    String imageURL = captureAndUploadPhoto();
    String alertMessage = "\xF0\x9F\x9A\xA8 Critical railway crack detected!\n" + location + "\nImage: " + imageURL;
    sendSMS(alertMessage);
  }

  delay(5000);
}
