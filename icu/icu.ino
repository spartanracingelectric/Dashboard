#include "config.h"
#include "leds.h"
#include "can.h"
#include "lcd.h"

#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

// LCD Object Initialization
// Args: (U8G2_R0/Rotate, SCK, MOSI, CS, A0/DC, RST) clock and data from SW_SPI: PICO_LCD_SPI_SCK, PICO_LCD_SPI_MOSI,
//U8G2_ST7565_NHD_C12864_F_4W_HW_SPI lcd_u8g2(U8G2_R2, PICO_LCD_SPI_CS, PICO_LCD_A0, PICO_LCD_RST);
U8G2_ST7565_NHD_C12864_F_4W_SW_SPI lcd_u8g2(U8G2_R2, PICO_LCD_SPI_SCK, PICO_LCD_SPI_MOSI, PICO_LCD_SPI_CS, PICO_LCD_A0, PICO_LCD_RST);

// LED Object Initialization
// Args: (MAX72XX_HARDWARE_TYPE, CS, NUM_MAX72XX_DEVICES)
// PAROLA_HW refers to an 8x8 LED matrix which we are sort of simulating

MD_MAX72XX leds_md = MD_MAX72XX(MAX72XX_HARDWARE_TYPE, PICO_LED_SPI_CS, 1);


#if (POWERTRAIN_TYPE == 'E')
float hv = 0.0f; 
float hvtemp = 0.0f;
float tps0percent = 0.0f;
float tps1percent = 0.0f;

#endif

void setup()
{
  pinMode(PICO_CAN_SPI_CS, OUTPUT);
  digitalWrite(PICO_CAN_SPI_CS, HIGH);
  pinMode(PICO_LED_SPI_CS, OUTPUT);
  digitalWrite(PICO_LED_SPI_CS, HIGH);

  //Serial.begin(115200);
#if (BOARD_REVISION == 'A')
  SPI.setSCK(PICO_CAN_SPI_SCK);
  SPI.setTX(PICO_CAN_SPI_MOSI);
  SPI.setRX(PICO_CAN_SPI_MISO);
  SPI.setCS(PICO_CAN_SPI_CS);
  SPI.begin();
#elif (BOARD_REVISION == 'B')
  pinMode(PICO_CAN_RST, OUTPUT);
  digitalWrite(PICO_CAN_RST, HIGH);
  SPI.setSCK(PICO_LED_SPI_SCK);
  SPI.setTX(PICO_LED_SPI_MOSI);
  SPI.setCS(PICO_LED_SPI_CS);
  SPI1.setSCK(PICO_CAN_SPI_SCK);
  SPI1.setTX(PICO_CAN_SPI_MOSI);
  SPI1.setRX(PICO_CAN_SPI_MISO);
  SPI1.setCS(PICO_CAN_SPI_CS);
  SPI.begin();
  SPI1.begin();
#endif

  // No need to initialize CABN here, as can.begin seems to hog the data
  // buffer which in turn stalls the MAX7219 and therefore the whole program

  // Initialize leds, pass U8G2 object pointer
  leds__init(&leds_md);

  // Initialize lcd, pass U8G2 object pointer
  lcd__init(&lcd_u8g2);

  //Non functional as clearBuffer in loop overwrites for now
  lcd__print_default_screen_template();
  leds__set_brightness(MAX_LED_BRIGHTNESS);
  leds__wake();
#if (BOARD_REVISION == 'B')
  can__start();
#endif

}

void loop()
{
  uint32_t curr_millis = millis();
  #if (BOARD_REVISION == 'A')
    can__start();
    delay(10);
  #endif
  //can__send_test();
  can__receive();

#if (POWERTRAIN_TYPE == 'E')
  hvtemp = can__get_hvtemp();
  tps0percent = can__get_tps0percent();
  tps1percent = can__get_tps1percent();

  if(tps0percent < 10 && tps1percent < 10){
    hv = can__get_hv(); // should be able to give no load voltage
  }


#endif


#if (BOARD_REVISION == 'A')
  can__stop();
#endif


#if (POWERTRAIN_TYPE == 'E')
    leds__safety_update_flash(hvtemp, curr_millis);
    lcd__update_screenE(hv, tps0percent, tps1percent, hvtemp, curr_millis);
    
#endif
  
}
