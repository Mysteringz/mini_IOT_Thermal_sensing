//
// Created by Zhifang L on 05/03/2026.
//

#ifndef REV2_4_THERMAL_NODE_LED_CONTROLLER_H
#define REV2_4_THERMAL_NODE_LED_CONTROLLER_H

#include <FastLED.h>

#define NUM_LEDS 4

#define LED_RED 0
#define LED_ORANGE 32
#define LED_YELLOW 64
#define LED_GREEN 96
#define LED_CYAN 128
#define LED_BLUE 160
#define LED_PURPLE 192
#define LED_PINK 224

extern CRGBArray<NUM_LEDS> leds;
extern uint8_t hue;

void led_init();
void led_set(uint8_t hue); // Array for 4 LEDs

#endif //REV2_4_THERMAL_NODE_LED_CONTROLLER_H
