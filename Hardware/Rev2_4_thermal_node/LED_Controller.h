//
// Created by Zhifang L on 05/03/2026.
//

#ifndef REV2_4_THERMAL_NODE_LED_CONTROLLER_H
#define REV2_4_THERMAL_NODE_LED_CONTROLLER_H

#include <FastLED.h>

#define NUM_LEDS 4
extern CRGBArray<NUM_LEDS> leds;
extern uint8_t hue;

void led_set(uint8_t hue);

#endif //REV2_4_THERMAL_NODE_LED_CONTROLLER_H