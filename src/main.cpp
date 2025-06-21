#include <Wire.h>
#include "giroscopio.h"
#include "proximitySensors.h" // Incluir el header de los sensores de proximidad
#include "web.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Arduino.h>
#include "lineDetector.h"
#include "laberinto.h"
#include "motores.h"

// Declarar externamente los handles de los semáforos definidos en giroscopio.cpp
extern SemaphoreHandle_t xSemaphoreGiroscopio;
extern SemaphoreHandle_t OffsetDataSemaphore;
TwoWire I2C2 = TwoWire(1);

void setup()
{
    Serial.begin(115200);
    Wire.end();

    vTaskDelay(pdMS_TO_TICKS(100));
    Wire.begin(21, 22);         // SDA=21, SCL=22
    I2C2.begin(26, 25, 400000); // SDA=26, SCL=25, 400kHz
    vTaskDelay(pdMS_TO_TICKS(500));

    
    xSemaphoreGiroscopio = xSemaphoreCreateMutex();
    OffsetDataSemaphore = xSemaphoreCreateMutex();
    
    if (xSemaphoreGiroscopio == NULL || OffsetDataSemaphore == NULL)
    {
        Serial.println("Error crítico: No se pudieron crear los semáforos en setup()");
        while (1)
        ;
    }
    
    // Crear la tarea para configurar el servidor web
    BaseType_t webTaskCreated = xTaskCreate(
        setupWebServer,
        "WebServerTask",
        8192,
        NULL,
        1,
        NULL);

    if (webTaskCreated != pdPASS)
    {
        Serial.println("Error crítico: No se pudo crear la tarea del servidor web");
        while (1)
            ;
    }

    vTaskDelay(pdMS_TO_TICKS(10000)); 

    BaseType_t gyroTaskCreated = xTaskCreate(
        initializeGiroscopio,
        "GiroscopioTask",
        5000,
        NULL,
        1,
        NULL);

    if (gyroTaskCreated != pdPASS)
    {
        Serial.println("Error crítico: No se pudo crear la tarea del giroscopio");
        while (1)
            ;
    }

    // Crear la tarea de los sensores de proximidad
    BaseType_t proximityTaskCreated = xTaskCreatePinnedToCore(
        proximityTask,   // Función de la tarea de proximitySensors.cpp
        "ProximityTask", // Nombre de la tarea
        10000,           // Tamaño de la pila (bytes)
        NULL,            // Sin parámetros
        1,               // Prioridad igual a las demás
        NULL,            // Sin handle
        1                // Ejecutar en Core 1
    );

    if (proximityTaskCreated != pdPASS)
    {
        Serial.println("Error crítico: No se pudo crear la tarea de sensores de proximidad");
        while (1)
            ;
    }
    // detector de linea
    initializeLineDetector();

    inicializarMotores();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar un segundo para asegurarse de que los motores estén listos
    // Iniciar laberinto
    BaseType_t mazeTaskCreated = xTaskCreate(
        initializeMaze,
        "MazeTask",
        8192,
        NULL,
        1,
        NULL);

    Serial.println("Setup completado. Todas las tareas creadas.");
}

void loop()
{
    // El loop principal puede quedar vacío para un sistema basado en tareas
    vTaskDelay(pdMS_TO_TICKS(1000)); // Solo para evitar watchdog triggers
}