#ifndef USE_HAL_DRIVER
#define USE_HAL_DRIVER
#endif
#ifndef STM32H753xx
#define STM32H753xx
#endif
#include "main.h"
#include "app_main.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"
#include "stm32h7xx_hal_conf.h"
#include "config.h"
#include "fdcan_bus.h"
#include "can_service.h"
#include "max7219.h"
#include "leds.h"
#include "lcd.h"
#include "timebase.h"



namespace lcd {
  void render_energy_bar_demo(uint32_t now_ms);
}

//External handles from CubeMX
extern SPI_HandleTypeDef hspi4;
//extern SPI_HandleTypeDef hspi1;
extern FDCAN_HandleTypeDef hfdcan1;
SPI_HandleTypeDef& hspi_lcd = hspi4;
SPI_HandleTypeDef& hspi_led = hspi4;

//Board Pins
LcdPins   LCD_PINS   {GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_5, GPIOA, GPIO_PIN_6};
Max7219Pins MAX_PINS {GPIOB, GPIO_PIN_12};

void app_main()
{

  // Drivers
  FdcanBus bus(&hfdcan1);
  Max7219 max(&hspi_led, MAX_PINS.cs_port, MAX_PINS.cs_pin);

  // Services
  leds::init(&max);
  leds::set_brightness(0x0F);
  leds::wake();
  leds::efficiency_on_can_ratio(1.0f);

  lcd::init();
  lcd::print_default_screen_template();

  if (!cansvc::init(bus)) {
    //lcd::show_error(msg); Need to put these 2 definitions in their respective codes 
    //leds::show_fault();
  }

  uint32_t t_led_safety = 0;

  while (1) {
    // CAN state polling instead of if else nesting
    // Option 1 just for demo of stuff on screen to test working screen
    cansvc::poll(bus);
    leds::efficiency_tick(now_ms());
    lcd::render_energy_bar_demo(now_ms());

    /* For option B need to uncomment
    lcd::update_screenE(
      cansvc::hv(),
      cansvc::tps0_percent(),
      cansvc::tps1_percent(),
      cansvc::hv_temp(),
      now_ms()
    );
    */

    if (every_ms(t_led_safety, 50)) {
      leds::safety_update_flash(cansvc::hv_temp(), now_ms());
      leds::lv(cansvc::lv());
    }
    static uint32_t t_can_test = 0; // to avoid starving other tasks
    if (every_ms(t_can_test, 100)) { 
      cansvc::send_test(bus); }

    //cansvc::send_test(bus);
  }
}
