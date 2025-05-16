#include "lineDetector.h"
#include <Arduino.h>

void initializeLineDetector()
{
    pinMode(LINE_SENSOR_PIN, INPUT);
    Serial.println("Line detector initialized");
}

bool ifLineDetected()
{
    int sensorValue = digitalRead(LINE_SENSOR_PIN);
    return (sensorValue == LOW);
}