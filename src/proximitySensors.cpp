#include "proximitySensors.h"
#include <VL53L0X.h>
#include <Arduino.h>

// Renombrar para claridad semántica
VL53L0X tofFront;
VL53L0X tofLeft;
VL53L0X tofRight;
VL53L0X *sensors[] = {&tofFront, &tofLeft, &tofRight};

extern TwoWire I2C2;

// Datos compartidos y semáforo (usar el nombre correcto)
static ProximityData proxData;
static SemaphoreHandle_t xSemaphoreProximityData = NULL; // Inicializar a NULL

// Constantes
#define PROXIMITY_SENSOR_TIMEOUT 250 // Timeout para la librería VL53L0X
#define PROXIMITY_TASK_DELAY_MS 50   // Reducir delay para lecturas más frecuentes

// --- Funciones Internas ---

bool initSingleSensor(VL53L0X &sensor, uint8_t xshutPin, uint8_t address, const char *sensorName)
{
    Serial.print("Inicializando ");
    Serial.print(sensorName);
    Serial.print(" (XSHUT: ");
    Serial.print(xshutPin);
    Serial.println(")...");

    digitalWrite(xshutPin, HIGH);  // Activar el sensor
    vTaskDelay(pdMS_TO_TICKS(20)); // Pequeña espera para estabilización (usar pdMS_TO_TICKS)
    sensor.setBus(&I2C2);          // Establecer el bus I2C para el sensor

    if (!sensor.init())
    {
        Serial.print("Error: Fallo al inicializar ");
        Serial.print(sensorName);
        Serial.print(" en pin XSHUT ");
        Serial.println(xshutPin);
        digitalWrite(xshutPin, LOW);
        return false;
    }

    sensor.setAddress(address);
    sensor.setTimeout(PROXIMITY_SENSOR_TIMEOUT);

    sensor.startContinuous(300);
    sensor.setMeasurementTimingBudget(250000); // 250 ms
    Serial.print(sensorName);
    Serial.print(" inicializado. Dirección: 0x");
    Serial.println(sensor.getAddress(), HEX);
    return true;
}

bool initializeSensorsInternal()
{
    pinMode(XSHUT1_PIN, OUTPUT);
    pinMode(XSHUT2_PIN, OUTPUT);
    pinMode(XSHUT3_PIN, OUTPUT);
    Serial.println("Pines XSHUT configurados como salida.");

    digitalWrite(XSHUT1_PIN, LOW);
    digitalWrite(XSHUT2_PIN, LOW);
    digitalWrite(XSHUT3_PIN, LOW);
    Serial.println("Todos los sensores ToF apagados inicialmente.");
    vTaskDelay(pdMS_TO_TICKS(100));

    // Define pins, addresses, and names corresponding to the sensors array
    // Ensure these constants (XSHUTx_PIN, TOFx_ADDR) are defined elsewhere in your project (e.g., proximitySensors.h)
    uint8_t xshutPins[] = {XSHUT1_PIN, XSHUT2_PIN, XSHUT3_PIN};
    uint8_t tofAddrs[] = {TOF1_ADDR, TOF2_ADDR, TOF3_ADDR}; // Example addresses: 0x30, 0x31, 0x32
    const char *sensorNames[] = {"Sensor Frontal", "Sensor Izquierdo", "Sensor Derecho"};
    size_t numSensors = sizeof(sensors) / sizeof(sensors[0]);

    Serial.println("Iniciando secuencia de inicialización individual de sensores ToF...");

    for (size_t i = 0; i < numSensors; ++i)
    {
        bool ok = initSingleSensor(*sensors[i], xshutPins[i], tofAddrs[i], sensorNames[i]);
        if (!ok)
        {
            Serial.print("Advertencia: Fallo la inicialización para ");
            Serial.print(sensorNames[i]);
            sensors[i] = nullptr;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Optional: Check if at least one sensor was initialized successfully
    bool one_ok = false;
    for (size_t i = 0; i < numSensors; ++i)
    {
        if (sensors[i] != nullptr)
        {
            one_ok = true;
            Serial.print("Sensor ");
            Serial.print(sensorNames[i]);
            Serial.println(" inicializado correctamente y disponible.");
        }
    }
    if (!one_ok)
    {
        Serial.println("Error crítico: Ningún sensor ToF pudo ser inicializado.");
        return false;
    }

    Serial.println("Inicialización de sensores ToF completada.");
    return true;
}

void readSensorsInternal()
{
    ProximityData tempData;
    tempData.frontDistance = 8191;
    tempData.leftDistance = 8191;
    tempData.rightDistance = 8191;
    uint16_t distances[3] = {0, 0, 0};
    for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); ++i)
    {
        if (sensors[i] != nullptr)
        {
            distances[i] = sensors[i]->readRangeContinuousMillimeters();
            if (sensors[i]->timeoutOccurred())
            {
                Serial.print("Error: Timeout en sensor ");
                Serial.println(i);
            }
        }
    }
    tempData.frontDistance = distances[0];
    tempData.leftDistance = distances[1];
    tempData.rightDistance = distances[2];

    if (xSemaphoreProximityData != NULL)
    {
        if (xSemaphoreTake(xSemaphoreProximityData, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            proxData = tempData;
            xSemaphoreGive(xSemaphoreProximityData);
        }
        else
        {
            Serial.println("Error: No se pudo tomar semáforo Proximity en readSensorsInternal");
        }
    }
}

// --- Funciones Públicas ---

// Tarea FreeRTOS para gestionar los sensores de proximidad
void proximityTask(void *parameters)
{
    if (xSemaphoreProximityData == NULL)
    {
        xSemaphoreProximityData = xSemaphoreCreateMutex();
    }

    if (xSemaphoreProximityData == NULL)
    {
        Serial.println("Error crítico: No se pudo crear el semáforo de proximidad.");
        vTaskDelete(NULL);
    }

    Serial.println("Iniciando inicialización de sensores ToF desde la tarea...");

    if (!initializeSensorsInternal())
    {
        Serial.println("Error crítico: Fallo en la inicialización de sensores ToF. Terminando tarea.");
        vTaskDelete(NULL);
    }
    Serial.println("Sensores ToF inicializados correctamente por la tarea.");

    while (true)
    {
        readSensorsInternal();
        vTaskDelay(pdMS_TO_TICKS(PROXIMITY_TASK_DELAY_MS));
    }
}

/**
 * @brief Obtiene los datos de los sensores de proximidad de forma thread-safe.
 * 
 * Esta función intenta obtener los datos más recientes de los sensores de proximidad
 * protegidos por un semáforo. Si no puede obtener el semáforo o éste no está
 * inicializado, devolverá valores predeterminados (8191) y registrará un mensaje
 * de error o advertencia.
 * 
 * @return ProximityData Estructura con los siguientes campos:
 *   - frontDistance: Distancia medida por el sensor frontal (en unidades de medición del sensor)
 *   - leftDistance: Distancia medida por el sensor izquierdo (en unidades de medición del sensor)
 *   - rightDistance: Distancia medida por el sensor derecho (en unidades de medición del sensor)
 * 
 * @note Si no se puede adquirir el semáforo, la función tiene un timeout de 10ms
 * @note El valor 8191 indica una lectura de distancia predeterminada o inválida
 */
ProximityData getProximityData()
{
    ProximityData tempData;
    tempData.frontDistance = 8191;
    tempData.leftDistance = 8191;
    tempData.rightDistance = 8191;

    if (xSemaphoreProximityData != NULL)
    {
        if (xSemaphoreTake(xSemaphoreProximityData, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            tempData = proxData;
            xSemaphoreGive(xSemaphoreProximityData);
        }
        else
        {
            Serial.println("Advertencia: No se pudo obtener semáforo en getProximityData. Devolviendo últimos datos o default.");
        }
    }
    else
    {
        Serial.println("Error: Semáforo Proximity no inicializado en getProximityData. Devolviendo defaults.");
    }
    return tempData;
}
