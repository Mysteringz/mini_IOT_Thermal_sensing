#pragma once
#include <Arduino.h>

void setupLoRa();
void handleLoRaStateMachine();
void prepareTxFrame(uint8_t port);

extern uint8_t appData[];
extern uint8_t appDataSize;
