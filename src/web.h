#ifndef WEB_H
#define WEB_H
#include <ESPAsyncWebServer.h>

void setupWebServer(void *parameter);
void handleGetGiroscopioData(AsyncWebServerRequest *request);
void handleGetSystemInfo(AsyncWebServerRequest *request);
void handleGetProximityData(AsyncWebServerRequest *request); // Handler para datos de proximidad
void handleGetLineDetection(AsyncWebServerRequest *request); // Handler para detección de línea negra

#endif