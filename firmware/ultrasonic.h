#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

// Simulate ultrasonic sensor readings based on time
float getSimulatedDepth() {
  static unsigned long lastUpdate = 0;
  static float depth = 0.5;

  if (millis() - lastUpdate > 3000) {
    lastUpdate = millis();

    // Cycle through simulated depths (in cm)
    // This sequence fakes: no crack → moderate → critical → repeat
    static int state = 0;
    switch (state) {
      case 0: depth = 0.8;  break;  // Non-critical crack
      case 1: depth = 1.7;  break;  // Moderate
      case 2: depth = 2.8;  break;  // Critical
      case 3: depth = 0.3;  break;  // Almost no crack
    }
    state = (state + 1) % 4;
  }

  return depth;
}

#endif
