#ifndef GIROSCOPIO_H
#define GIROSCOPIO_H

#include <MPU6050.h>
#include <freertos/FreeRTOS.h> // Añadido para tipos FreeRTOS
#include <freertos/task.h>     // Añadido para funciones de tareas FreeRTOS

struct GiroscopioData
{
    float angleX, angleY, angleZ;
    float gyroX, gyroY, gyroZ;
    float accelX, accelY, accelZ;
    float temperature;
};

struct OffsetData
{
    int16_t gyroXOffset, gyroYOffset, gyroZOffset;
    int16_t accelXOffset, accelYOffset, accelZOffset;
};

/**
 * @brief Estructura para almacenar información de diagnóstico de una tarea FreeRTOS.
 */
struct GiroTaskInfoData {
    eTaskState eCurrentState;           // Estado actual de la tarea.
    configSTACK_DEPTH_TYPE usStackHighWaterMark; // Marca de agua alta: mínimo espacio LIBRE que ha tenido el stack (en palabras).
    
};

void initializeGiroscopio(void *parameters);
GiroscopioData getGiroscopioData();
OffsetData getOffsetData();
void resetData();
void readGiroSensors();
void calibrateSensors();
GiroTaskInfoData getGiroTaskInfoData();
#endif