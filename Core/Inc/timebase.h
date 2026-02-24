#pragma once
#include "stm32h7xx_hal.h"

inline uint32_t now_ms() { return HAL_GetTick(); }
inline bool every_ms(uint32_t& last, const uint32_t period) {
  const uint32_t t = HAL_GetTick();
  if (t - last >= period) { last = t; return true; }
  return false;
}
