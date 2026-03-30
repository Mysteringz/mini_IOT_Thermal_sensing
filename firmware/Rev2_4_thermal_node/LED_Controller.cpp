//
// Created by Zhifang L on 05/03/2026.
//

#include "LED_Controller.h"

#include <HardwareSerial.h>

CRGBArray<NUM_LEDS> leds;

uint8_t hue = 0;

void led_init() {
    Serial.println("LED Initializing");
    FastLED.addLeds<NEOPIXEL,LED_INPUT_PIN>(leds, NUM_LEDS);
    delay(1000);
    // FastLED.setBrightness(255);
    led_set(LED_PINK); // Because I like pink
    delay(1000);      // Optional: small delay after initialization
}

void led_set(uint8_t hue) {
    leds[0] = CHSV(hue,255,255);
    // leds[1] = CHSV(hue,255,255);
    // leds[2] = CHSV(hue,255,255);
    // leds[3] = CHSV(hue,255,255);

    FastLED.show();
}