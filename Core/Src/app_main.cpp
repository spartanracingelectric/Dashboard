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
#include "apa102.h"
#include "leds.h"
#include "lcd.h"
#include "timebase.h"



namespace lcd {
  void render_energy_bar_demo(uint32_t now_ms);
}

//External handles from CubeMX
extern SPI_HandleTypeDef hspi4;
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
SPI_HandleTypeDef& hspi_lcd = hspi4;

//Board Pins
LcdPins   LCD_PINS   {GPIOA, GPIO_PIN_4, GPIOA, GPIO_PIN_5, GPIOA, GPIO_PIN_6};

void app_main()
{

  // Drivers
  FdcanBus bus_vcu(&hfdcan1);   // pins 2/3 → VCU @ 500 kbit/s
  FdcanBus bus_bms(&hfdcan2);   // pins 4/5 → BMS @ 1 Mbit/s
  Apa102Chain bar_chain(LT_BAR_DI_GPIO_Port,   LT_BAR_DI_Pin,
                        LT_BAR_CI_GPIO_Port,   LT_BAR_CI_Pin,   12);
  Apa102Chain left_chain(LT_LEFT_DI_GPIO_Port, LT_LEFT_DI_Pin,
                         LT_LEFT_CI_GPIO_Port, LT_LEFT_CI_Pin,  3);
  Apa102Chain right_chain(LT_RIGHT_DI_GPIO_Port, LT_RIGHT_DI_Pin,
                          LT_RIGHT_CI_GPIO_Port, LT_RIGHT_CI_Pin, 3);

  leds::init(&bar_chain, &left_chain, &right_chain);
  leds::set_brightness(31);
  leds::wake();
  LCD_init();


  cansvc::init_vcu(bus_vcu);
  cansvc::init_bms(bus_bms);

  uint32_t t_heartbeat = 0;

  while (1) {
    // Process incoming CAN from both buses
    cansvc::poll(bus_vcu);
    cansvc::poll(bus_bms);

    // Update LED bar from efficiency data
    leds::efficiency_tick(now_ms());
    LCD_demoCodeTest();

    // Heartbeat
    if (every_ms(t_heartbeat, 200)) {
      cansvc::send_test(bus_vcu);
    }
  }
}
