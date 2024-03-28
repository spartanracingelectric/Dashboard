
#include "config.h"
#include "leds.h"
#include "can.h"
#include "lcd.h"
#include "rotary.h"
#include <string>
using namespace std; 

U8G2_ST7565_NHD_C12864_F_4W_SW_SPI lcd_u8g2(U8G2_R2, PICO_LCD_SPI_SCK, PICO_LCD_SPI_MOSI, PICO_LCD_SPI_CS, PICO_LCD_A0, PICO_LCD_RST);

// LED Object Initialization
// Args: (MAX72XX_HARDWARE_TYPE, CS, NUM_MAX72XX_DEVICES)
// PAROLA_HW refers to an 8x8 LED matrix which we are sort of simulating

MD_MAX72XX leds_md = MD_MAX72XX(MAX72XX_HARDWARE_TYPE, PICO_LED_SPI_CS, 1);

float hv = 0.0f;
float hvCurr = 0.0f;
float soc = 0.0f;
float lv = 0.0f;
float tps0volt = 0.0f;
float tps0calib = 0.0f;
float tps1volt = 0.0f;
float tps1calib = 0.0f;
float bps0volt = 0.0f;
float bps0calib = 0.0f;
int cell_over_volt = 0;
int pack_over_volt = 0;
int monitor_comm = 0;
int precharge = 0;
int failedthermistor = 0;
float maxtorque = 0.0f;
float hvtemp = 0.0f;
float hvlow = 0.0f;
int regenmode = 0;
float drsEnable = 0.0f;
int drsMode = 0;
float launchReady = 0.0f;
float launchStatus = 0.0f;
int hvil = 0;
int bspd = 0;

// diagnostics ---------------------------
uint16_t rpm = 0;
uint8_t cellfault = 0;
uint8_t cellwarn = 0;
uint8_t bmsstate = 0;


void setup()
{

  // CAN Pins
  pinMode(PICO_CAN_SPI_CS, OUTPUT);
  digitalWrite(PICO_CAN_SPI_CS, HIGH);
  pinMode(PICO_LED_SPI_CS, OUTPUT);
  digitalWrite(PICO_LED_SPI_CS, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  
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
  leds__init(&leds_md);
  lcd__init(&lcd_u8g2);
  lcd_welcome_screen();
  delay(1000);
  lcd__print_default_screen_template();
  leds__set_brightness(MAX_LED_BRIGHTNESS);
  leds__wake();
  can__start();
}

void loop()
{
  uint32_t curr_millis = millis(); // switch time var
  can__receive();

  hv = can__get_hv();
  hvCurr = can__get_hv_current();
  hvil = can__get_hvil();
  bspd = can__get_bspd();
  soc = can__get_soc();
  hvtemp = can__get_hvtemp();
  lv = can__get_lv();
  hvlow = can__get_hvlow();
  regenmode = can__get_regenmode();
  drsEnable = can__get_drsEnable();
  drsMode = can__get_drsMode();
  launchReady = can__get_launchReady();
  launchStatus = can__get_launchStatus();
  tps0volt = can__get_tps0voltage();
  tps0calib = can__get_tps0calibmax();
  tps1volt = can__get_tps1voltage();
  tps1calib = can__get_tps1calibmax();
  bps0volt = can__get_bps0voltage();
  bps0calib = can__get_bps0calibmax();
  cell_over_volt = can__get_cellovervoltage();
  pack_over_volt = can__get_packovervoltage();
  monitor_comm = can__get_monitorcommfault();
  precharge = can__get_prechargefault();
  failedthermistor = can__get_failedthermistor();
  maxtorque = can__get_maxtorque();
// diagnostics --------------------------------- // don't work
  cellfault = can__get_bms_fault();
  cellwarn = can__get_bms_warn();
  bmsstate = can__get_bms_stat();
  rpm = can__get_rpm();

//     leds__safety_update_flash(hvlow, hvtemp, curr_millis);
  lcd__update_screenE(hvil, hv, soc, lv, hvlow, hvtemp, hvCurr, drsMode, regenmode, 
    launchReady, tps0volt, tps0calib, tps1volt, tps1calib, bps0volt, 
    bps0calib, cell_over_volt, pack_over_volt, monitor_comm, precharge, failedthermistor, maxtorque, curr_millis);
  // leds__rpm_update_tach(rpm);
  // leds__lv(lv,displayScreen);
  // leds__debug(displayScreen);
  leds__hvtemp(hvtemp);
  // leds__hvil(hvil, hv, displayScreen);
  // leds__bspd(bspd, displayScreen);
}
