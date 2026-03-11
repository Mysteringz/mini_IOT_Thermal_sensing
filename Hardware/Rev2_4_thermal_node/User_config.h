//
// Created by Zhifang L on 04/03/2026.
//

#ifndef MINI_IOT_THERMAL_SENSING_USER_CONFIG_H
#define MINI_IOT_THERMAL_SENSING_USER_CONFIG_H

// Used for debugging only, should be false during deployment
static const bool ACTIVATE_SERVER = false;

// Needed for ESP-NOW protocol or debugging servers
static const char PROGMEM host[] = "iot2";
static const char PROGMEM ssid[] = "IOT_TEAM_WIFI";
static const char PROGMEM password[] = "12345678";

#endif //MINI_IOT_THERMAL_SENSING_USER_CONFIG_H
