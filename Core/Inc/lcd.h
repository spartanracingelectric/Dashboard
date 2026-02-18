// lcd.h (EVE backend)
#pragma once

#include <cstdint>
#include "config.h"

namespace lcd {

// Initializes the EVE display (clocks/PLL/touch via your HAL) and theme colors.
void init();

// Clears the full screen for a single frame (mostly cosmetic on EVE).
void clear();

// Simple splash like the old U8g2 template (“Created by: Aryan Singh”).
void print_default_screen_template();

// Clears a logical section of the UI (indices defined in eve .cpp).
void clear_section(uint8_t sect);

// Individual “print_*” updaters (kept for API parity with the U8g2 port).
// Each updates the cached string and draws a frame.
void print_hv(float hv);
void print_tps0percent(float p);
void print_tps1percent(float p);
void print_hvtemp(float t);

// “No Load Voltage” dynamic value (hvlow in the original U8g2 code).
void print_hvlow(float v);

// Diagnostics readouts (cellfault/cellwarn/bmsstate) and RPM diag.
void diagnostics(uint8_t cellfault, uint8_t cellwarn, uint8_t bmsstate);
void print_rpm_diag(uint16_t rpm);

// Periodic UI updater (rate-limited with LCD_UPDATE_MS).
void update_screenE(float hv, float tps0, float tps1, float hvtemp, uint32_t now_ms);
void render_energy_bar_demo(uint32_t now_ms);

} // namespace lcd
