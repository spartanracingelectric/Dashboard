#pragma once
#include "stm32h7xx_hal_conf.h"
extern "C"{
#include "stm32h753xx.h"
#include "stm32h7xx_hal_fdcan.h"
#include <cstdint>
using std::uint32_t;
}

// ===== Driver parameters & cadences (from original config) =====
constexpr uint32_t LCD_UPDATE_MS              = 500;     // screen cadence  (orig)  :contentReference[oaicite:20]{index=20}
constexpr uint32_t HVLOW_THRESHOLD_FLASH_MS   = 750;     // LED flash period :contentReference[oaicite:21]{index=21}
constexpr uint32_t HVTEMP_THRESHOLD_FLASH_MS  = 750;     // LED flash period :contentReference[oaicite:22]{index=22}

constexpr float    LV_WARNING_THRESHOLD       = 12.0f;   // LV warning       :contentReference[oaicite:23]{index=23}
constexpr float    HVTEMP_LIMIT_C             = 50.0f;   // new: unify LED flash limit (was hardcoded 50) :contentReference[oaicite:24]{index=24}

// ===== CAN bus (classic 500k) =====
constexpr uint32_t CAN_BITRATE = 500000;                 // 500 kbit/s (orig) :contentReference[oaicite:25]{index=25}

// ===== LCD pins (choose your ports/pins) =====
// TODO: set real GPIO ports/pins and wire up in CubeMX
struct LcdPins {
  GPIO_TypeDef* cs_port;  uint16_t cs_pin;
  GPIO_TypeDef* a0_port;  uint16_t a0_pin;   // D/C
  GPIO_TypeDef* rst_port; uint16_t rst_pin;
};
extern LcdPins LCD_PINS;

// ===== SPI handles (provided by CubeMX/MX_SPIx_Init) =====
extern SPI_HandleTypeDef& hspi_lcd;

// ===== FDCAN handle (provided by CubeMX) =====
extern FDCAN_HandleTypeDef hfdcan1;
