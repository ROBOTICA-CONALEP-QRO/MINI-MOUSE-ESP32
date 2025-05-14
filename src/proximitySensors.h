#ifndef PROXIMITYSENSORS_H
#define PROXIMITYSENSORS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define XSHUT1_PIN 17
#define XSHUT2_PIN 16
#define XSHUT3_PIN 4

#define TOF1_ADDR 0x30
#define TOF2_ADDR 0x31
#define TOF3_ADDR 0x32

struct ProximityData
{
    uint16_t frontDistance;
    uint16_t leftDistance;
    uint16_t rightDistance;
};

void proximityTask(void *parameters);

ProximityData getProximityData();

#endif
