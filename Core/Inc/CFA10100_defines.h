
#ifndef __CFA10100_DEFINES_H__
#define __CFA10100_DEFINES_H__

/*
 * STM32H753ZI + BT817/BT818 EVE board config for CFA800480Ex-050Sx
 * - Replaces Arduino/AVR pin defines and PORTx bit macros with STM32 HAL GPIO.
 * - Keeps 800x480 timing and BT81x options from the original header.
 * - Set your GPIO pins/ports in the section labeled "Pin mapping".
 */

#include "stm32h7xx_hal.h"

/* ---- EVE device selection ---- */
#define EVE_DEVICE           (BT817)
#include "EVE_defines.h"

/* ---- Demo feature toggles (unchanged; you can turn these off) ---- */
#define PROGRAM_FLASH_FROM_USD (0)
#if (0 == PROGRAM_FLASH_FROM_USD)
  #define BMP_DEMO             (1)
  #define   BMP_SCROLL         (0)
  #define SOUND_DEMO           (0)
  #define   SOUND_VOICE        (1)
  #define   SOUND_PLAY_TIMES   (1)
  #define LOGO_DEMO            (0)
  #define LOGO_PNG_0_ARGB2_1   (1)
  #define BOUNCE_DEMO          (0)
  #define MARBLE_DEMO          (0)
  #define TOUCH_DEMO           (1)
  #define VIDEO_DEMO           (1)
  #define   VIDEO_BOUNCE       (1)
  #define FLASH_VIDEO_WIDTH  ((uint16_t)400)
  #define FLASH_VIDEO_HEIGHT ((uint16_t)240)
  #define FLASH_SECTOR_MARBLE (1UL)
  #define FLASH_LENGTH_MARBLE (0UL)
  #define FLASH_SECTOR_SPLASH (1UL)
  #define FLASH_LENGTH_SPLASH (96000UL)
  #define FLASH_SECTOR_CLOUDS (25UL)
  #define FLASH_LENGTH_CLOUDS (-2147450880UL)
  #define FLASH_SECTOR_ICE_FPV_512x300 (25UL)
  #define FLASH_LENGTH_ICE_FPV_512x300 (12882892UL)
#else
  #define BMP_DEMO             (0)
  #define   BMP_SCROLL         (0)
  #define SOUND_DEMO           (0)
  #define   SOUND_VOICE        (0)
  #define   SOUND_PLAY_TIMES   (0)
  #define LOGO_DEMO            (0)
  #define LOGO_PNG_0_ARGB2_1   (0)
  #define BOUNCE_DEMO          (0)
  #define MARBLE_DEMO          (0)
  #define TOUCH_DEMO           (0)
  #define VIDEO_DEMO           (0)
#endif

/* ---- Debug level ---- */
#define DEBUG_LEVEL (DEBUG_GEEK)

/* ---- SD build flag (unchanged) ---- */
#if ((0 != SOUND_DEMO) || (0 != BMP_DEMO) || (0 != MARBLE_DEMO) || (0 != PROGRAM_FLASH_FROM_USD))
  #define BUILD_SD           (1)
#else
  #define BUILD_SD           (0)
#endif

/* =============================================================================
 *                           Pin mapping (STM32 HAL)
 * -----------------------------------------------------------------------------
 * Set these to match your CubeMX project (or #define them in a central board.h).
 * Default placeholders below use example ports/pins on NUCLEO-H753ZI.
 * =============================================================================
 */
#ifndef EVE_INT_GPIO_Port
  #define EVE_INT_GPIO_Port      GPIOC
  #define EVE_INT_Pin            GPIO_PIN_13
#endif

#ifndef EVE_PD_NOT_GPIO_Port
  #define EVE_PD_NOT_GPIO_Port   GPIOB
  #define EVE_PD_NOT_Pin         GPIO_PIN_0   /* PD_N (reset) */
#endif

#ifndef EVE_CS_NOT_GPIO_Port
  #define EVE_CS_NOT_GPIO_Port   GPIOB
  #define EVE_CS_NOT_Pin         GPIO_PIN_1   /* SPI CS */
#endif

#ifndef SD_CS_GPIO_Port
  #define SD_CS_GPIO_Port        GPIOB
  #define SD_CS_Pin              GPIO_PIN_2   /* optional: SD CS */
#endif

#ifndef DEBUG_LED_GPIO_Port
  #define DEBUG_LED_GPIO_Port    GPIOE
  #define DEBUG_LED_Pin          GPIO_PIN_2
#endif

/* ---- Fast GPIO macros using HAL (replace AVR PORTx ops) ---- */
#define CLR_EVE_PD_NOT()        HAL_GPIO_WritePin(EVE_PD_NOT_GPIO_Port, EVE_PD_NOT_Pin, GPIO_PIN_RESET)
#define SET_EVE_PD_NOT()        HAL_GPIO_WritePin(EVE_PD_NOT_GPIO_Port, EVE_PD_NOT_Pin, GPIO_PIN_SET)
#define CLR_EVE_CS_NOT()        HAL_GPIO_WritePin(EVE_CS_NOT_GPIO_Port, EVE_CS_NOT_Pin, GPIO_PIN_RESET)
#define SET_EVE_CS_NOT()        HAL_GPIO_WritePin(EVE_CS_NOT_GPIO_Port, EVE_CS_NOT_Pin, GPIO_PIN_SET)
#define CLR_SD_CS_NOT()         HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET)
#define SET_SD_CS_NOT()         HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET)
#define CLR_DEBUG_LED()         HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET)
#define SET_DEBUG_LED()         HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_SET)

/* (Optional) input read helper for EVE_INT */
#define READ_EVE_INT()          (HAL_GPIO_ReadPin(EVE_INT_GPIO_Port, EVE_INT_Pin) == GPIO_PIN_SET)

/* =============================================================================
 *                         EVE clock/touch/display config
 * =============================================================================
 */
#define EVE_CLOCK_SOURCE     (EVE_CLOCK_SOURCE_EXTERNAL)
#define EVE_CLOCK_MUL        (EVE_EXTERNAL_CLOCK_MUL_x6_72MHz)
#define EVE_CLOCK_SPEED      ((uint32_t)(EVE_CLOCK_MUL & 0x1F) * (uint32_t)12000000)

/* Touch: pick the controller used on your CFA800480Ex-050Sx variant. */
#define EVE_TOUCH_TYPE       (EVE_TOUCH_CAPACITIVE)
/* Common variants are FT5316 or GT911. Defaulting to FT5316; change if needed. */
#define EVE_TOUCH_CAP_DEVICE (EVE_CAP_DEV_FT5316)

#define EVE_PEN_UP_BUG_FIX   (0)
/* Set to 1 to force on-device calibration and print the matrix. */
#define EVE_TOUCH_CAL_NEEDED (0)

/* RGB pin order from PCB layout (0 = R:G:B normal) */
#define LCD_SWIZZLE      (0x0)

/* PCLK polarity: 1 = data changes on falling, captured on rising (common) */
#define LCD_PCLKPOL      (1)

/* ---- BT81x pin-drive table (keep; PROGMEM neutralized for non-AVR) ---- */
#ifndef PROGMEM
  #define PROGMEM
#endif

#if ((EVE_DEVICE == BT815) || (EVE_DEVICE == BT816) || (EVE_DEVICE == BT817) || (EVE_DEVICE == BT818))
  #define PIN_DRIVE_TABLE const uint8_t Pin_Drive_Table[22] PROGMEM = { \
    (EVE_PINDRIVE_GPIO_0_HR      | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_GPIO_1_HR      | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_GPIO_2_HR      | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_GPIO_3_HR      | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_DISP_LR        | EVE_PINDRIVE_LR_1p2mA), \
    (EVE_PINDRIVE_DE_LR          | EVE_PINDRIVE_LR_1p2mA), \
    (EVE_PINDRIVE_VSYNC_HSYNC_LR | EVE_PINDRIVE_LR_1p2mA), \
    (EVE_PINDRIVE_PCLK_LR        | EVE_PINDRIVE_LR_3p6mA), \
    (EVE_PINDRIVE_BACKLIGHT_LR   | EVE_PINDRIVE_LR_1p2mA), \
    (EVE_PINDRIVE_RGB_LR         | EVE_PINDRIVE_LR_1p2mA), \
    (EVE_PINDRIVE_AUDIO_L_HR     | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_INT_N_HR       | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_CTP_RST_N_HR   | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_CTP_SCL_HR     | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_CTP_SDA_HR     | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_SPI_DATA_HR    | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_SPIM_SCLK_HR   | EVE_PINDRIVE_HR_10mA), \
    (EVE_PINDRIVE_SPIM_SS_N_HR   | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_SPIM_MISO_HR   | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_SPIM_MOSI_HR   | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_SPIM_IO2_HR    | EVE_PINDRIVE_HR_5mA), \
    (EVE_PINDRIVE_SPIM_IO3_HR    | EVE_PINDRIVE_HR_5mA)  \
  }
#else
  #define LCD_DRIVE_10MA   (0)
#endif

/* Spread spectrum / dither options */
#define LCD_PCLK_CSPREAD (0)
#define LCD_DITHER       (0)

/* ---- Pixel clock divisor (BT81x runs at 72 MHz w/ x6 external clock) ----
 * 0=disable; 2 -> ~36 MHz PCLK (good for 800x480@~60Hz)
 */
#define LCD_PCLK         (2)

/* ---- 800x480 timing (derived from original ILI6122 example) ----
 * Tune if your panel needs different porch/sync widths.
 */
#define HPX   (800)    /* active width */
#define HSW   (4)      /* hsync width */
#define HBP   (8)      /* back porch (includes HSW per original comments) */
#define HFP   (8)      /* front porch */
#define HPP   (178)    /* padding so EVE has time each line */

#define LCD_WIDTH   (HPX)
#define LCD_HSYNC0  (HFP)
#define LCD_HSYNC1  (HFP+HSW)
#define LCD_HOFFSET (HFP+HSW+HBP)
#define LCD_HCYCLE  (HPX+HFP+HSW+HBP+HPP)

#define VLH   (480)   /* active height */
#define VS    (4)     /* vsync width (lines) */
#define VBP   (8)     /* back porch (includes VS) */
#define VFP   (8)     /* front porch */
#define VLP   (1)     /* line padding */

#define LCD_HEIGHT  (VLH)
#define LCD_VSYNC0  (VFP)
#define LCD_VSYNC1  (VFP+VS)
#define LCD_VOFFSET (VFP+VS+VBP)
#define LCD_VCYCLE  (VLH+VFP+VS+VBP+VLP)

/* ---- Safety checks matching original header intent ---- */
#if (EVE_TOUCH_TYPE == EVE_TOUCH_NONE)
  #if (0 != TOUCH_DEMO)
    #error Cannot enable touch demo for a non-touch display.
  #endif
#endif

#if ((EVE_TOUCH_TYPE == EVE_TOUCH_RESISTIVE) && \
     ((EVE_DEVICE == FT801) || (EVE_DEVICE == FT811) || (EVE_DEVICE == FT813) || \
      (EVE_DEVICE == FT815) || (EVE_DEVICE == FT817)))
  #error Cannot specify EVE_TOUCH_RESISTIVE for an EVE_DEVICE that only supports capacitive touch.
#endif

#if ((EVE_TOUCH_TYPE == EVE_TOUCH_CAPACITIVE) && \
     ((EVE_DEVICE == FT800) || (EVE_DEVICE == FT810) || (EVE_DEVICE == FT812) || \
      (EVE_DEVICE == FT816) || (EVE_DEVICE == FT818)))
  #error Cannot specify EVE_TOUCH_CAPACITIVE for an EVE_DEVICE that only supports resistive touch.
#endif

/* Debug helpers enabled when DEBUG_LEVEL != DEBUG_NONE */
#if (DEBUG_LEVEL != DEBUG_NONE)
  #define REMOTE_BACKLIGHT_DEBUG (1)
#endif

#if ((0 != LOGO_DEMO) && (1 == LOGO_PNG_0_ARGB2_1))
  #define DEBUG_COPROCESSOR_RESET (0)
#endif

#endif /* __CFA10100_DEFINES_H__ */
