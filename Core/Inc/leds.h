#pragma once
#include <cstdint>
#include "config.h"

class Apa102Chain;

constexpr uint8_t NUM_LED_BAR   = 12;
constexpr uint8_t NUM_LED_LEFT  = 3;
constexpr uint8_t NUM_LED_RIGHT = 3;

namespace leds {
// lifecycle
void init(Apa102Chain* bar, Apa102Chain* left, Apa102Chain* right);
void wake();

// shift indicator (bar chain)
void enable_shift();
void disable_shift();
void disable_all_solid();
void disable_half_solid(bool firstHalf);
void toggle_overrev();
void toggle_half(bool firstHalf);
void toggle_revlim();
void set_brightness(uint8_t value);

// safety warnings (left/right chains)
void lv(float lv);
void hvtemp(float hvtemp);
void safety_update_flash(float hvtemp, uint32_t now_ms);

// efficiency bar (bar chain)
void efficiency_on_can_ratio(float ratio);
void efficiency_on_can_error(float signed_err);
void efficiency_tick(uint32_t now_ms);
}
