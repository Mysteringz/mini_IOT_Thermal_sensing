//
// Created by Zhifang L on 05/03/2026.
//

#include "LED_Controller.h"

CRGBArray<NUM_LEDS> leds;

uint8_t hue = 0;

void led_set(uint8_t hue) {
    leds[0] = CHSV(hue,255,255);
    leds[1] = CHSV(hue,255,255);
    leds[2] = CHSV(hue,255,255);
    leds[3] = CHSV(hue,255,255);
}