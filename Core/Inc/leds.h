// leds.h
#pragma once
#include <cstdint>
#include "max7219.h"
#include "config.h"

constexpr uint8_t NUM_LED_SOLID = 10;
constexpr uint8_t NUM_LED_RGB   = 6;

extern const uint8_t PIN_LED_SOLID[NUM_LED_SOLID][2];
extern const uint8_t PIN_LED_RGB_R[NUM_LED_RGB][2];
extern const uint8_t PIN_LED_RGB_G[NUM_LED_RGB][2];
extern const uint8_t PIN_LED_RGB_B[NUM_LED_RGB][2];

namespace leds {
// lifecycle
void init(Max7219* dev);
void wake();


void enable_shift();
void disable_shift();
void disable_all_solid();
void disable_half_solid(bool firstHalf);
void toggle_overrev();
void toggle_half(bool firstHalf);
void toggle_revlim();
void set_brightness(uint8_t value);
void lv(float lv);             
void hvtemp(float hvtemp);
void safety_update_flash(float hvtemp, uint32_t now_ms);
void efficiency_on_can_ratio(float ratio);
void efficiency_on_can_error(float signed_err);
void efficiency_tick(uint32_t now_ms);
}
