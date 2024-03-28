#ifndef LEDS_H_
#define LEDS_H_

#include <Arduino.h>
#include <MD_MAX72xx.h>
#include "config.h"

/*---------------------------------------------------------------------------/
/ LED MAP TO MAX7219 ROW/COLUMN
/---------------------------------------------------------------------------*/
// Solid state LED map
// 10 array rows per Solid State LED, 2 array columns for MAX7219 row & column # respectively
const uint8_t NUM_LED_SOLID = 10;
const uint8_t NUM_LED_RGB = 6;
const uint8_t PIN_LED_SOLID[NUM_LED_SOLID][2] = {{0,0}, {0,1}, {0,2}, {0,3}, {0,4},
                                                {1,0}, {1,1}, {1,2}, {1,3}, {1,4}};

// RGB LED map (Todo)
// {Color of LED, which LED}
// 6 array rows per RGB LED number (red only), 2 array columns per MAX7219 {row, column}
const uint8_t PIN_LED_RGB_R[NUM_LED_RGB][2] = {{2,0}, {2,2}, {2,4},
                                            {2,1}, {2,3}, {2,5}};
// 6 array rows per RGB LED number (green only), 2 array columns per MAX7219 {row, column}
const uint8_t PIN_LED_RGB_G[NUM_LED_RGB][2] = {{3,0}, {3,2}, {3,4},
                                            {3,1}, {3,3}, {3,5}};
// 6 array rows per RGB LED number (blue only), 2 array columns per MAX7219 {row, column}
const uint8_t PIN_LED_RGB_B[NUM_LED_RGB][2] = {{4,0}, {4,2}, {4,4},
                                            {4,1}, {4,3}, {4,5}};

void leds__init(MD_MAX72XX *leds_ptr);
void leds__wake();
void leds__start();
void leds__rpm_update_tach(uint16_t rpm);
//void leds__rpm_update_flash(uint16_t rpm, uint8_t gear, uint32_t curr_millis);
void leds__safety_update_flash(float hvlow, float hvtemp, uint32_t curr_millis);
void leds__set_brightness(uint8_t brightness_value);
//void leds__oilpress(float oilpress);
void leds__debug(int displayScreen);
void leds__lv(float lv, int displayScreen);
void leds__hvtemp(float hvtemp, int displayScreen);
void leds__drsEnable(float drsEnable, int displayScreen);
void leds__launchReady(float launchReady, int displayScreen);
void leds__hvlow(float hvlow);
void leds__regenMode(int regenmode, int displayScreen);
void leds__regenModeSet(int regenmode, int displayScreen, uint8_t mode);
void leds__hvil(int hvil, float hv);

//create LEDs Update function that takes CAN data and changes LEDs


#endif /* LEDS_H_ */
