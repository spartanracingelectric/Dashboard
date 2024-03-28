#ifndef CONFIG_H_
#define CONFIG_H_

/********************************************************************/
/*  INSTRUMENT CLUSTER - SETUP CONFIGURATION FILE (for ports, etc)  */
/*                                                                  */
/********************************************************************/
// Set to 1 if you want to print the input data over serial
#define SERIAL_DEBUG_EN 0


/*---------------------------------------------------------------------------/
/ DRIVER PARAMETERS
/---------------------------------------------------------------------------*/

#define DEFAULT_SHIFT_THRESHOLD_RPM 11500
#define G12_SHIFT_THRESHOLD_RPM 10356
#define G23_SHIFT_THRESHOLD_RPM 10239
#define G34_SHIFT_THRESHOLD_RPM 10278
#define G45_SHIFT_THRESHOLD_RPM 9498
//#define G56_SHIFT_THRESHOLD_RPM 11000 // unused atm

#define OVERREV_THRESHOLD_RPM_ADDER 500

//#define OVERREV_THRESHOLD_RPM (SHIFT_THRESHOLD_RPM+500)
#define REVLIM_THRESHOLD_RPM 13000
//#define SHIFT_THRESHOLD_FLASH_MS
#define OVERREV_THRESHOLD_FLASH_MS 250
#define REVLIM_THRESHOLD_FLASH_MS 100

#define HVLOW_THRESHOLD_FLASH_MS 750
#define HVTEMP_THRESHOLD_FLASH_MS 750

#define LCD_UPDATE_MS 500

/*---------------------------------------------------------------------------/
/ PIN INITIALIZATIONS
/---------------------------------------------------------------------------*/
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


#define TESTING_SCREEN 0
#define MINIMALIST_SCREEN 1
#define DEBUG_BMS_SCREEN 2
#define DEBUG_VCU_SCREEN 3
#define MENU_SCREEN 4         // menu screen always has to be last - when assigning row to screen after clicking it's 1 to 1 instead of many if statements

#define NUMBER_OF_SCREENS 5   // change when add or subtract screens
#define NUMBER_OF_ROWS 5      // number of rows at menu screen

#define DEBOUNCE_TIME 600
#define LOCK_TIME 100

#define HOLD_TIME 1000 //Button hold time for bottom buttons on default screen


#define CANBUS_SPEED 500UL * 1000UL //500 kbit/s
#define CAN_RPM_ADDR 0x0A5
#define CAN_TPS0 0x500
#define CAN_TPS1 0x501
#define CAN_BPS0 0x502
#define CAN_SAFETY 0x506
#define CAN_LV 0x507
#define CAN_LV_ADDR 0x507

// Need to validate
#define CAN_REGEN_ADDR 0x508 // Need to validate. technically done, test LCD
#define CAN_LAUNCH_ADDR 0x50B // Need to validate. technically done, test LCD
#define CAN_DRS_ADDR 0x50C // Need to validate
#define CAN_BMS_FAULT_ADDR 0x602 // dec: 1538
#define CAN_BMS_WARN_ADDR 0x604 // dec: 1540
#define CAN_BMS_STAT_ADDR 0x610 // dec: 1552 // system status (should be idle - 1)

#define CAN_HV_ADDR 0x620
#define CAN_HV_CURRENT_ADDR 0x620
#define CAN_HV_ADDR 0x620 // dec: 1568
#define CAN_SOC_ADDR 0x621
#define CAN_HVLOW_ADDR 0x622
#define CAN_BAT_TEMP_ADDR 0x623
#define LV_WARNING_THRESHOLD 12.0f

#define WHEEL_DIAMETER_IN 16.0f
#define WHEEL_CIRCUMFERENCE_M (WHEEL_DIAMETER_IN * 0.0254 * 3.141f)
#define METERS_SEC_TO_MPH  2.237f


/*---------------------------------------------------------------------------/
/ CAN ADDRESSES
/---------------------------------------------------------------------------*/
/* CAN packet addresses based on the spreadsheet */
#define SOC_ADDR 0x627
#define BAT_TEMP_ADDR 0x628
#define RPM_ADDR 0x0A5
#define WSS_ADDR 0x505      // Interpolated WSS address
//#define REGEN_ADDR 0x508 // using for Regen mode
#define HV_ADDR 0x623
#define LV_ADDR 0x700
#define TS_ADDR 0x701       // Timestamp Address (-> DAQ)
#define REGEN1_ADDR 0x702       // (-> VCU)
#define REGEN2_ADDR 0x703       // (-> VCU)
#define REGEN3_ADDR 0x704
#define FAULT_ADDR 0x0AB
#define MAX72XX_HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_LED_BRIGHTNESS 0xFF

