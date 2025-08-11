#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

#ifndef ULTRASONIC_TRIG_PIN
#define ULTRASONIC_TRIG_PIN 4
#endif

#ifndef ULTRASONIC_ECHO_PIN
#define ULTRASONIC_ECHO_PIN 34
#endif

inline void initUltrasonic() {
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delay(50);
}

inline float getDepthCm(uint32_t timeout_us = 30000) {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, timeout_us);
  if (duration == 0) return -1.0f;

  return duration / 58.0f; // cm
}

inline float getSimulatedDepth() { return getDepthCm(); }

#endif // ULTRASONIC_H
