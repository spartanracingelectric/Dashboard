#ifndef LCD_H_
#define LCD_H_

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include "config.h"

// Display Screen Status
// 0 for Driver
// 1 for Menu
// 2 for Individual Menu Screens
// 3 for RPM Threshold




// Fonts and other Setup
void lcd__init(U8G2_ST7565_NHD_C12864_F_4W_SW_SPI *lcd_ptr); // changed from SW -> HW
void lcd__clear_screen();
void lcd__print8(uint8_t x, uint8_t y, const char *str);                 //Print str with Helvetica Bold Size 10
void lcd__print14(uint8_t x, uint8_t y, const char *str);                 //Print str with Helvetica Bold Size 14
void lcd__print18(uint8_t x, uint8_t y, char *str);                 //Print str with Helvetica Bold Size 18
void lcd__print24(uint8_t x, uint8_t y, char *str);                 //Print str with Helvetica Bold Size 24
void lcd__print_default_screen_template();
void lcd__clear_section(uint8_t section);

// C car
//void lcd__print_rpm(uint16_t rpm);
//void lcd__print_gear(uint8_t gear);
//void lcd__print_oilpress(float oilpress); // Oil coolant? pressure // float or uint8
// E car
void lcd__print_hv(float hv); // accumulator voltage
void lcd__print_wattemp(uint8_t watertemp); // Water Coolant Temp
void lcd__print_motortemp(float motortemp); // Motor temp

// E & C car
void lcd__print_lv(float lv); // low voltage battery soc
void lcd__print_tps0voltage(float tps0, int displayScreen);
void lcd__print_tps1voltage(float tps1, int displayScreen);
void lcd__print_hvtemp(float hvtemp); // Accumulator/Engine temperature
void lcd__print_hvlow(float hvlow);

// 
//void lcd_print__tiracctemp(uint8_t tiracctemp); // Tire temperatures multiple variables?

// Menu stuff
void lcd__menu(void);
void lcd__highlight_screen(uint8_t row, const char* screen);
void lcd__print_screen(uint8_t selection, uint8_t row, char screen); // 5 row + Back template
void lcd__drs_screen(uint8_t drs);

// diagnostics screen
void lcd__diagnostics(uint8_t cellfault, uint8_t cellwarn, uint8_t bmsstate);
void lcd__print_rpm_diag(uint16_t rpm); // rpm diagnostics for ev

void lcd__update_screen(uint16_t rpm, uint8_t gear, float lv, float oilpress, uint8_t drs, uint32_t curr_millis_lcd);
void lcd__update_screenE(float hv, float soc, float lv, float hvlow, float hvtemp, float tps0volt, float tps0percent, float tps1volt, float tps1percent, uint32_t curr_millis_lcd);

#endif /* LCD_H_ */
