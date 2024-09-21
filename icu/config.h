#ifndef CONFIG_H_
#define CONFIG_H_

/********************************************************************/
/*  INSTRUMENT CLUSTER - SETUP CONFIGURATION FILE (for ports, etc)  */
/*                                                                  */
/********************************************************************/

// Board Revision, select:
// 'A' for Rev A
// 'B' for Rev B
//#define BOARD_REVISION 'A'
#define BOARD_REVISION 'B'

//#define POWERTRAIN_TYPE 'C'
#define POWERTRAIN_TYPE 'E'

// Set to 1 if you want to print the input data over serial
#define SERIAL_DEBUG_EN 0


/*---------------------------------------------------------------------------/
/ DRIVER PARAMETERS
/---------------------------------------------------------------------------*/

#define HVLOW_THRESHOLD_FLASH_MS 750
#define HVTEMP_THRESHOLD_FLASH_MS 750

#define LCD_UPDATE_MS 500

/*---------------------------------------------------------------------------/
/ PIN INITIALIZATIONS
/---------------------------------------------------------------------------*/

//  Rev A
#if (BOARD_REVISION == 'A')
//  LCD
#define PICO_LCD_SPI_CS     2
#define PICO_LCD_SPI_MOSI   4
#define PICO_LCD_SPI_SCK    5
#define PICO_LCD_A0         3
#define PICO_LCD_RST        U8X8_PIN_NONE
//  LEDS, MAX7219
#define PICO_LED_SPI_SCK    18
#define PICO_LED_SPI_MOSI   19
#define PICO_LED_SPI_CS     13
//CAN, MCP2515
#define PICO_CAN_SPI_SCK    18
#define PICO_CAN_SPI_MOSI   19
#define PICO_CAN_SPI_MISO   16
#define PICO_CAN_SPI_CS     17

//  Rev B
#elif (BOARD_REVISION == 'B')
//  LCD
#define PICO_LCD_SPI_CS     5
#define PICO_LCD_SPI_MOSI   3
#define PICO_LCD_SPI_SCK    2
#define PICO_LCD_A0         4
#define PICO_LCD_RST        1
//  LEDS, MAX7219
#define PICO_LED_SPI_SCK    18
#define PICO_LED_SPI_MOSI   19
#define PICO_LED_SPI_CS     17
//CAN, MCP2515
#define PICO_CAN_SPI_SCK    10
#define PICO_CAN_SPI_MOSI   11
#define PICO_CAN_SPI_MISO   12
#define PICO_CAN_SPI_CS     13
#define PICO_CAN_INT        14
#define PICO_CAN_RST        15
#endif

/*---------------------------------------------------------------------------/
/ BUTTON PINS AND Screen CONSTANTS
/---------------------------------------------------------------------------*/
#define BUT1 9 //PINC0
#define BUT2 10 //PINC1
#define BUT3 11 //PINC2
#define BUT4 12 //PINC3

#define DEFAULT_SCREEN 0
#define MENU_SCREEN 1
#define DIAGNOSTICS_SCREEN 2
#define RPM_SCREEN 3
//#define OPTIONY_SCREEN 4

#define DEBOUNCE_TIME 600
#define LOCK_TIME 100

#define HOLD_TIME 1000 //Button hold time for bottom buttons on default screen

//  CAN BUS
/////////////////////
/* #if (POWERTRAIN_TYPE == 'C')
//#define CANBUS_SPEED 1000UL * 1000UL //1 Mbit/s
#define CANBUS_SPEED 500UL * 1000UL //500 kbit/s
#define CAN_RPM_ADDR    0x640
#define CAN_GEAR_ADDR   0x703
// lv, drs, oil pressure, coolant temp
*/

#if (POWERTRAIN_TYPE == 'E')
#define CANBUS_SPEED 500UL * 1000UL //500 kbit/s
#define CAN_RPM_ADDR 0x0A5
#define CAN_TPS0 0x500 // dec: 1280
#define CAN_TPS1 0x501 // dec: 1281
#define CAN_LV_ADDR 0x507
#define CAN_HV_ADDR 0x620 // dec: 1568
#define CAN_BAT_TEMP_ADDR 0x623 // dec: 1571

#endif

//  LOW VOLTAGE
/////////////////////
#define LV_WARNING_THRESHOLD 12.0f

//  WHEEL SPEEDS
/////////////////////
//Wheel diameter in inches
#define WHEEL_DIAMETER_IN 16.0f
#define WHEEL_CIRCUMFERENCE_M (WHEEL_DIAMETER_IN * 0.0254 * 3.141f)
#define METERS_SEC_TO_MPH  2.237f

/*---------------------------------------------------------------------------/
/ LOW VOLTAGE
/---------------------------------------------------------------------------*/

#define LV_OFFSET .8f


#define MAX72XX_HARDWARE_TYPE MD_MAX72XX::PAROLA_HW

#ifndef BOARD_REVISION
#error "Select a board revision"
#endif
#ifndef POWERTRAIN_TYPE
#error "Select a powertrain type (C for Combustion or E for Electric)"
#endif

/*---------------------------------------------------------------------------/
/ HELPFUL MACROS
/---------------------------------------------------------------------------*/
#define MAX_LED_BRIGHTNESS 0xF

#endif /* CONF_H_ */
