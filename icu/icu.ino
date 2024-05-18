#include "config.h"
#include "leds.h"
#include "can.h"
#include "lcd.h"
#include "rotary.h"
#include <string>
using namespace std; 

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

/* #if (POWERTRAIN_TYPE == 'C')
uint16_t rpm = 0;
uint8_t gear = 0;
float oilpress = 0;
float lv = 0.0f;
uint8_t drs = 0;
*/

#if (POWERTRAIN_TYPE == 'E')
float hv = 0.0f;
float hvCurr = 0.0f;
float soc = 0.0f;
float lv = 0.0f;
float tps0 = 0.0f;
float hvtemp = 0.0f;
float hvlow = 0.0f;
int regenmode = 0;
float drsEnable = 0.0f;
int drsMode = 0;
float launchReady = 0.0f;
float launchStatus = 0.0f;
int torque = 0;
float tpscalib = 0;
float bpscalib = 0;

// diagnostics ---------------------------
uint16_t rpm = 0;
uint8_t cellfault = 0;
uint8_t cellwarn = 0;
uint8_t bmsstate = 0;

// rotary ---------------------------
int lastStateCLK;  // Read the initial state of CLK
int currentStateCLK;
bool currentStateSW;
int currentStateDT;

// lcd ---------------------------
int displayScreen = 0;
int prevDisplayScreen = -1;
int rowCount = 0;
int prevRowCount = -1;
#endif

void setup()
{

  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT);
  int lastStateCLK = digitalRead(CLK);  // Read the initial state of CLK

  // CAN Pins
  pinMode(PICO_CAN_SPI_CS, OUTPUT);
  digitalWrite(PICO_CAN_SPI_CS, HIGH);
  pinMode(PICO_LED_SPI_CS, OUTPUT);
  digitalWrite(PICO_LED_SPI_CS, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  
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

  // No need to initialize CAB here, as can.begin seems to hog the data
  // buffer which in turn stalls the MAX7219 and therefore the whole program

  // Initialize leds, pass U8G2 object pointer
  leds__init(&leds_md);

  // Initialize lcd, pass U8G2 object pointer
  lcd__init(&lcd_u8g2);

  //Non functional as clearBuffer in loop overwrites for now
  lcd_welcome_screen();
  lcd__print_default_screen_template();
  leds__set_brightness(MAX_LED_BRIGHTNESS);
  leds__wake();
#if (BOARD_REVISION == 'B')
  can__start();
#endif
  /*
    //LED Sniffing code
    delay(5000);
    for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      Serial.println(i);
      Serial.println(j);
      Serial.println();
      leds_md.setPoint(i,j,true);
      delay(1000);
      leds_md.setPoint(i,j,false);
    }
    }
  */
}

//Test values for proof of concept
//uint8_t cnt = 0;
//uint16_t test_vals[] = {109, 250, 1259, 2400, 3658, 4815, 4405, 4623, 6042, 7404, 9480, 11400, 12000, 13004};
//uint16_t test_vals[] = {14600};

void loop()
{
  // rotary position variables
  currentStateCLK = digitalRead(CLK);
  currentStateSW = digitalRead(SW);
  currentStateDT = digitalRead(DT);
  uint32_t curr_millis = millis(); // switch time var
  
  #if (BOARD_REVISION == 'A')
    can__start();
    delay(10);
  #endif
  //can__send_test();
  can__receive();

  displayRotary (currentStateCLK, currentStateSW, currentStateDT, lastStateCLK, displayScreen, rowCount, torque);

/* #if(POWERTRAIN_TYPE == 'C')
  rpm = can__get_rpm();
  gear = can__get_gear();
  oilpress = can__get_oilpress();
  lv = can__get_lv();
  drs = can__get_drs();
*/
#if (POWERTRAIN_TYPE == 'E')
  tpscalib = can__get_vcuTps();
  bpscalib = can__getvcuBps();

  hv = can__get_hv();
  hvCurr = can__get_hv_current();
  soc = can__get_soc();
//  wattemp = can__get_wattemp(); // no can
  hvtemp = can__get_hvtemp();
  lv = can__get_lv();
  hvlow = can__get_hvlow();
  regenmode = can__get_regenmode();
  drsEnable = can__get_drsEnable();
  drsMode = can__get_drsMode();
  launchReady = can__get_launchReady();
  launchStatus = can__get_launchStatus();

  
  
// diagnostics --------------------------------- // don't work
  cellfault = can__get_bms_fault();
  cellwarn = can__get_bms_warn();
  bmsstate = can__get_bms_stat();
  rpm = can__get_rpm();
#endif

//  drs = can__get_dr);

#if (BOARD_REVISION == 'A')
  can__stop();
#endif

  // placeholder values. uncomment when needed
//  rpm = 10000;
//  gear = 1;
  //oilpress = 15; // most likely float - reference hv or lv
  //drs = 3;
  //lv = 14.540510;
//  hv = 250.81430;
//  soc = 97;
//  hvtemp = 51.8234;
    //hvlow = 3.2f;
    // hvtemp = 52.3f;

  //lcd__print_rpm(rpm, curr_millis);
/* #if (POWERTRAIN_TYPE == 'C')
    leds__rpm_update_flash(rpm, gear, curr_millis);
    lcd__update_screen(rpm, gear, lv, oilpress, drs, curr_millis);
*/

#if (POWERTRAIN_TYPE == 'E')
//     leds__safety_update_flash(hvlow, hvtemp, curr_millis);
    lcd__update_screenE(hv, soc, lv, hvlow, hvtemp, hvCurr, drsMode, regenmode, launchReady, tps0, displayScreen, rowCount, prevDisplayScreen, prevRowCount,torque, currentStateCLK, lastStateCLK, currentStateDT, tpscalib, bpscalib, curr_millis);
    leds__rpm_update_tach(rpm);
    leds__drsEnable(drsEnable);
    leds__launchReady(launchStatus);
    //leds__lv(lv);
    // leds__regenMode(regenmode);
    leds__hvtemp(hvtemp);    
#endif
  //delay(500);
}
