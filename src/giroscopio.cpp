#include "giroscopio.h"
#include <MPU6050.h>
#include <Arduino.h>

// --- Constantes Definidas ---
#define GYRO_SENSITIVITY 131.0f
#define ACCEL_SENSITIVITY 16384.0f
#define COMPLEMENTARY_FILTER_ALPHA 0.96f
#define DYNAMIC_ALPHA_THRESHOLD 1.0f // Umbral para activar alpha dinámico
#define DYNAMIC_ALPHA_VALUE 0.7f
#define NOISE_THRESHOLD 0.5f
#define CALIBRATION_SAMPLES 1000
#define ACCEL_Z_GRAVITY_OFFSET 16384 // Asume FS=+-16g, ajustar si cambia la escala
#define TEMP_CONVERSION_DIVISOR 340.0f
#define TEMP_CONVERSION_OFFSET 36.53f
#define MPU_DELAY_MS 10
#define MAX_DT 0.2f // Límite superior para dt

MPU6050 mpu;
GiroscopioData giroscopioData;
OffsetData offsetData;
SemaphoreHandle_t xSemaphoreGiroscopio;
SemaphoreHandle_t OffsetDataSemaphore;

// Variables globales (considerar encapsular en una clase en el futuro)
float angleX, angleY, angleZ;
bool mpuInitialized = false;
unsigned long prevMillis = 0;

// Environment variables
float dt = 0;

// FREERTOS VARIABLES

// --- Renombrada función ---
void initializeGiroscopio(void *parameter)
{
    // Verificar que los semáforos fueron creados previamente en setup()
    if (xSemaphoreGiroscopio == NULL || OffsetDataSemaphore == NULL)
    {
        Serial.println("Error: Semáforos no inicializados correctamente antes de iniciar la tarea.");
        vTaskDelete(NULL); // No continuar si los semáforos no existen
    }

    Serial.println("Inicializando MPU6050...");

    // Intentar inicializar el MPU6050 con reintentos
    bool connected = false;
    for (int attempts = 0; attempts < 5; attempts++)
    {
        Serial.print("Intento ");
        Serial.print(attempts + 1);
        Serial.println(" de conectar con MPU6050...");

        mpu.initialize();
        if (mpu.testConnection())
        {
            connected = true;
            Serial.println("MPU6050 conectado correctamente!");
            break;
        }

        Serial.println("Fallo en la conexión MPU6050, reintentando...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if (!connected)
    {
        Serial.println("MPU6050 no conectado después de múltiples intentos");
        vTaskDelete(NULL);
    }
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
    mpu.setDLPFMode(MPU6050_DLPF_BW_5);
    prevMillis = millis();
    calibrateSensors();
    mpuInitialized = true;
    while (true)
    {
        readGiroSensors();
        vTaskDelay(MPU_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void calibrateSensors()
{
    Serial.println("Calibrando sensores");
    Serial.println("Mantener el sensor inmóvil...");
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    long gyro_x_sum = 0, gyro_y_sum = 0, gyro_z_sum = 0;
    long accel_x_sum = 0, accel_y_sum = 0, accel_z_sum = 0;

    int16_t ax, ay, az, gx, gy, gz;

    for (int i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        gyro_x_sum += gx;
        gyro_y_sum += gy;
        gyro_z_sum += gz;
        accel_x_sum += ax;
        accel_y_sum += ay;
        accel_z_sum += (az - ACCEL_Z_GRAVITY_OFFSET);
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }

    int16_t avg_gyro_x = gyro_x_sum / CALIBRATION_SAMPLES;
    int16_t avg_gyro_y = gyro_y_sum / CALIBRATION_SAMPLES;
    int16_t avg_gyro_z = gyro_z_sum / CALIBRATION_SAMPLES;
    int16_t avg_accel_x = accel_x_sum / CALIBRATION_SAMPLES;
    int16_t avg_accel_y = accel_y_sum / CALIBRATION_SAMPLES;
    int16_t avg_accel_z = accel_z_sum / CALIBRATION_SAMPLES;

    if (OffsetDataSemaphore != NULL)
    {
        if (xSemaphoreTake(OffsetDataSemaphore, portMAX_DELAY))
        {
            offsetData.gyroXOffset = avg_gyro_x;
            offsetData.gyroYOffset = avg_gyro_y;
            offsetData.gyroZOffset = avg_gyro_z;
            offsetData.accelXOffset = avg_accel_x;
            offsetData.accelYOffset = avg_accel_y;
            offsetData.accelZOffset = avg_accel_z;
            xSemaphoreGive(OffsetDataSemaphore);
            Serial.println("Calibración completa. Offsets guardados.");
        }
        else
        {
            Serial.println("Error al tomar semáforo de offset en calibración.");
        }
    }
    else
    {
        Serial.println("Semáforo de offset no inicializado en calibración.");
    }
}

void resetData()
{
    if (mpuInitialized)
    {
        if (xSemaphoreGiroscopio != NULL && xSemaphoreTake(xSemaphoreGiroscopio, portMAX_DELAY))
        {
            giroscopioData.angleX = 0;
            giroscopioData.angleY = 0;
            giroscopioData.angleZ = 0;
            xSemaphoreGive(xSemaphoreGiroscopio);
        }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

GiroscopioData getGiroscopioData()
{
    GiroscopioData temp = {0};
    if (mpuInitialized && xSemaphoreGiroscopio != NULL)
    {
        if (xSemaphoreTake(xSemaphoreGiroscopio, (TickType_t)10))
        {
            temp = giroscopioData;
            xSemaphoreGive(xSemaphoreGiroscopio);
        }
        else
        {
            Serial.println("Advertencia: No se pudo obtener semáforo giroscopio en getGiroscopioData");
        }
    }
    return temp;
}

OffsetData getOffsetData()
{
    OffsetData temp = {0};
    if (mpuInitialized && OffsetDataSemaphore != NULL)
    {
        if (xSemaphoreTake(OffsetDataSemaphore, (TickType_t)10))
        {
            temp = offsetData;
            xSemaphoreGive(OffsetDataSemaphore);
        }
        else
        {
            Serial.println("Advertencia: No se pudo obtener semáforo offset en getOffsetData");
        }
    }
    return temp;
}

void readGiroSensors()
{
    if (!mpuInitialized)
        return;

    OffsetData currentOffsets;
    if (OffsetDataSemaphore != NULL)
    {
        if (xSemaphoreTake(OffsetDataSemaphore, (TickType_t)5))
        {
            currentOffsets = offsetData;
            xSemaphoreGive(OffsetDataSemaphore);
        }
        else
        {
            Serial.println("Error: No se pudo obtener semáforo offset en readGiroSensors. Saltando lectura.");
            return;
        }
    }
    else
    {
        Serial.println("Error: Semáforo offset no inicializado en readGiroSensors.");
        return;
    }

    GiroscopioData temp;
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    unsigned long currentMillis = millis();
    dt = (currentMillis - prevMillis) / 1000.0f;
    prevMillis = currentMillis;

    if (dt > MAX_DT)
        dt = MAX_DT;
    if (dt <= 0)
        dt = 1.0f / (1000.0f / MPU_DELAY_MS);

    float gyroX_raw = gx - currentOffsets.gyroXOffset;
    float gyroY_raw = gy - currentOffsets.gyroYOffset;
    float gyroZ_raw = gz - currentOffsets.gyroZOffset;

    float accelX_raw = ax - currentOffsets.accelXOffset;
    float accelY_raw = ay - currentOffsets.accelYOffset;
    float accelZ_raw = az - currentOffsets.accelZOffset;

    float gyroX = gyroX_raw / GYRO_SENSITIVITY;
    float gyroY = gyroY_raw / GYRO_SENSITIVITY;
    float gyroZ = gyroZ_raw / GYRO_SENSITIVITY;

    float accelX = accelX_raw / ACCEL_SENSITIVITY;
    float accelY = accelY_raw / ACCEL_SENSITIVITY;
    float accelZ = (az - ACCEL_Z_GRAVITY_OFFSET - currentOffsets.accelZOffset) / ACCEL_SENSITIVITY;

    gyroX = (abs(gyroX) < NOISE_THRESHOLD) ? 0.0f : gyroX;
    gyroY = (abs(gyroY) < NOISE_THRESHOLD) ? 0.0f : gyroY;
    gyroZ = (abs(gyroZ) < NOISE_THRESHOLD) ? 0.0f : gyroZ;

    float accelMagnitude = sqrt(pow(accelX, 2) + pow(accelY, 2) + pow(accelZ, 2));
    float accelAngleX = 0.0f;
    float accelAngleY = 0.0f;
    if (accelMagnitude > 0.1)
    {
        accelAngleX = atan2(accelY, sqrt(pow(accelX, 2) + pow(accelZ, 2))) * RAD_TO_DEG;
        accelAngleY = atan2(-accelX, sqrt(pow(accelY, 2) + pow(accelZ, 2))) * RAD_TO_DEG;
    }

    float dynamicAlpha = COMPLEMENTARY_FILTER_ALPHA;
    if (abs(gyroX) > DYNAMIC_ALPHA_THRESHOLD || abs(gyroY) > DYNAMIC_ALPHA_THRESHOLD)
    {
        dynamicAlpha = DYNAMIC_ALPHA_VALUE;
    }

    if (isnan(angleX))
        angleX = accelAngleX;
    if (isnan(angleY))
        angleY = accelAngleY;

    angleX = dynamicAlpha * (angleX + gyroX * dt) + (1.0f - dynamicAlpha) * accelAngleX;
    angleY = dynamicAlpha * (angleY + gyroY * dt) + (1.0f - dynamicAlpha) * accelAngleY;

    angleZ += gyroZ * dt;

    temp.angleX = angleX;
    temp.angleY = angleY;
    temp.angleZ = angleZ;
    temp.gyroX = gyroX;
    temp.gyroY = gyroY;
    temp.gyroZ = gyroZ;
    temp.accelX = accelX;
    temp.accelY = accelY;
    temp.accelZ = accelZ;
    temp.temperature = mpu.getTemperature() / TEMP_CONVERSION_DIVISOR + TEMP_CONVERSION_OFFSET;

    if (xSemaphoreGiroscopio != NULL)
    {
        if (xSemaphoreTake(xSemaphoreGiroscopio, (TickType_t)5))
        {
            giroscopioData = temp;
            xSemaphoreGive(xSemaphoreGiroscopio);
        }
        else
        {
            Serial.println("Advertencia: No se pudo obtener semáforo giroscopio en readGiroSensors para actualizar datos.");
        }
    }
    else
    {
        Serial.println("Error: Semáforo giroscopio no inicializado en readGiroSensors.");
    }
}

GiroTaskInfoData getGiroTaskInfoData()
{
    GiroTaskInfoData info;
    info.eCurrentState = eTaskGetState(NULL);
    info.usStackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);

    return info;


}