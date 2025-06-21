#include <Arduino.h>
#include "web.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "giroscopio.h"
#include "proximitySensors.h"
#include <LittleFS.h>
#include <ESPmDNS.h>
#include "lineDetector.h"

//const char *ssid = "Totalplay-F4A3";
//const char *password = "F4A3D4DB82qNSUMy";
const char *ssid = "esp32s";
const char *password = "12345678";

static AsyncWebServer server(80);

void setupWebServer(void *parameter)
{
    if (!LittleFS.begin(true, "/storage", 1, "storage"))
    {
        Serial.println("Error al montar el sistema de archivos LittleFS");
        vTaskDelete(NULL);
    }

    // --- Inicio: Listar archivos LittleFS ---
    Serial.println("Listando archivos en LittleFS:");
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.println(file.size());
        file = root.openNextFile();
    }
    root.close();
    Serial.println("--- Fin: Listado LittleFS ---");

    //WiFi.mode(WIFI_STA);
    //WiFi.begin(ssid, password);
    /*while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        Serial.println("Conectando a WiFi...");
    }*/
   //Crear red wifi
    WiFi.mode(WIFI_AP_STA);
    if(WiFi.softAP(ssid, password)) {
        Serial.println("------------------------------------------");
        Serial.println("Red WiFi creada exitosamente");
        Serial.println("Nombre de la red: " + String(ssid));
        Serial.println("Contraseña: " + String(password));
        Serial.println("------------------------------------------");
        Serial.println("Para conectarte:");
        Serial.println("1. Conecta tu dispositivo a la red '" + String(ssid) + "'");
        Serial.println("2. Abre un navegador web");
        Serial.println("3. Navega a http://192.168.4.1 o http://esp32s.local");
        Serial.println("------------------------------------------");
    } else {
        Serial.println("Error al crear el punto de acceso WiFi");
    }
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Dirección IP del punto de acceso: ");
    Serial.println(myIP);

    // mDNS
    if (!MDNS.begin("esp32s"))
    {
        Serial.println("Error al iniciar mDNS, usando IP");
    }

    // Primero configuramos los endpoints de API
    server.on("/getMPUdata", HTTP_GET, handleGetGiroscopioData);
    server.on("/getSystemInfo", HTTP_GET, handleGetSystemInfo);
    server.on("/getProximityData", HTTP_GET, handleGetProximityData);
    server.on("/getLineDetection", HTTP_GET, handleGetLineDetection);  // Nuevo endpoint para detección de línea
    
    // Después configuramos el servidor de archivos estáticos
    // El archivo por defecto para la ruta "/" será "index.html"
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    
    server.begin();

    // La configuración está completa, la tarea puede eliminarse
    vTaskDelete(NULL);
}

void handleGetGiroscopioData(AsyncWebServerRequest *request)
{
    GiroscopioData mpuData = getGiroscopioData();
    OffsetData offsetsData = getOffsetData();
    
    // Angulos, aceleración, giroscopio y temperatura
    String jsonResponse = "{";
    jsonResponse += "\"angleX\": " + String(mpuData.angleX) + ",\"angleY\": " + String(mpuData.angleY) + ",\"angleZ\": " + String(mpuData.angleZ) + ",";
    jsonResponse += "\"ax\": " + String(mpuData.accelX) + ",\"ay\": " + String(mpuData.accelY) + ",\"az\": " + String(mpuData.accelZ) + ",";
    jsonResponse += "\"gx\": " + String(mpuData.gyroX) + ",\"gy\": " + String(mpuData.gyroY) + ",\"gz\": " + String(mpuData.gyroZ) + ",";
    jsonResponse += "\"temperature\": " + String(mpuData.temperature) + ",";
    
    // Offsets
    jsonResponse += "\"offsets\": {";
    jsonResponse += "\"gyroX\": " + String(offsetsData.gyroXOffset) + ",";
    jsonResponse += "\"gyroY\": " + String(offsetsData.gyroYOffset) + ",";
    jsonResponse += "\"gyroZ\": " + String(offsetsData.gyroZOffset) + ",";
    jsonResponse += "\"accelX\": " + String(offsetsData.accelXOffset) + ",";
    jsonResponse += "\"accelY\": " + String(offsetsData.accelYOffset) + ",";
    jsonResponse += "\"accelZ\": " + String(offsetsData.accelZOffset) + "}";
    
    jsonResponse += "}";
    
    request->send(200, "application/json", jsonResponse);
}

void handleGetSystemInfo(AsyncWebServerRequest *request)
{
    // Obtener información del sistema
    uint32_t freeHeap = ESP.getFreeHeap() / 1024;     // Convertir a KB
    uint32_t totalHeap = ESP.getHeapSize() / 1024;    // Convertir a KB
    uint32_t usedHeap = totalHeap - freeHeap;         // Calcular usado en KB
    
    uint32_t totalFlash = ESP.getFlashChipSize() / 1024;  // Flash total en KB
    uint32_t usedFlash = ESP.getSketchSize() / 1024;      // Flash usado por sketch en KB
    uint32_t freeFlash = totalFlash - usedFlash;          // Flash libre (aproximado) en KB
    
    String chipModel = ESP.getChipModel();
    uint32_t chipCores = ESP.getChipCores();
    uint32_t cpuFreqMHz = ESP.getCpuFreqMHz();
    
    // Sistema
    String jsonResponse = "{";
    jsonResponse += "\"freeHeap\": " + String(freeHeap) + ",";
    jsonResponse += "\"totalHeap\": " + String(totalHeap) + ",";
    jsonResponse += "\"usedHeap\": " + String(usedHeap) + ",";
    jsonResponse += "\"freeFlash\": " + String(freeFlash) + ",";
    jsonResponse += "\"totalFlash\": " + String(totalFlash) + ",";
    jsonResponse += "\"usedFlash\": " + String(usedFlash) + ",";
    jsonResponse += "\"chipModel\": \"" + chipModel + "\",";
    jsonResponse += "\"chipCores\": " + String(chipCores) + ",";
    jsonResponse += "\"cpuFreqMHz\": " + String(cpuFreqMHz) + "";
    jsonResponse += "}";
    
    request->send(200, "application/json", jsonResponse);
}

void handleGetProximityData(AsyncWebServerRequest *request)
{
    ProximityData proximityData = getProximityData();
    
    String jsonResponse = "{";
    jsonResponse += "\"front\": " + String(proximityData.frontDistance) + ",";
    jsonResponse += "\"left\": " + String(proximityData.leftDistance) + ",";
    jsonResponse += "\"right\": " + String(proximityData.rightDistance) + "";
    jsonResponse += "}";
    
    request->send(200, "application/json", jsonResponse);
}

// Nueva función para manejar la detección de línea
void handleGetLineDetection(AsyncWebServerRequest *request)
{
    bool lineDetected = ifLineDetected();
    
    String jsonResponse = "{";
    jsonResponse += "\"lineDetected\": " + String(lineDetected ? "true" : "false");
    jsonResponse += "}";
    
    request->send(200, "application/json", jsonResponse);
}