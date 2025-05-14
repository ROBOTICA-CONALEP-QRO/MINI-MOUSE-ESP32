#ifndef WEB_H
#define WEB_H
#include <ESPAsyncWebServer.h>

void setupWebServer(void *parameter);
void handleGetGiroscopioData(AsyncWebServerRequest *request);
void handleGetSystemInfo(AsyncWebServerRequest *request);
void handleGetProximityData(AsyncWebServerRequest *request); // Nuevo handler para datos de proximidad

#endif