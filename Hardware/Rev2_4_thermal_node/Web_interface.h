//
// Created by Zhifang L on 04/03/2026.
//

#ifndef MINI_IOT_THERMAL_SENSING_WEB_INTERFACE_H
#define MINI_IOT_THERMAL_SENSING_WEB_INTERFACE_H

#include <Arduino.h>
#include "User_config.h"

// Website Style
extern const String style;

// Login page
extern const String loginIndex;

// Server Index Page
extern const String serverIndex;

bool beginMDNS(const char* host);

// Starts Web Server for LoRaWAN interface
void setupLoRaInterfaceServer();

// Process one pending HTTP client request.
void handleLoRaInterfaceClient();

#endif //MINI_IOT_THERMAL_SENSING_WEB_INTERFACE_H
