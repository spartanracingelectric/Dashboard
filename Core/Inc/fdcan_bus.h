// fdcan_bus.hpp
#pragma once
#include "stm32h7xx_hal.h"
extern "C"{
#include "stm32h7xx_hal_fdcan.h"
}
#include <cstdint>
#include <functional>

struct CanFrame {
  uint32_t id;         // 11-bit std id
  uint8_t  len;
  uint8_t  data[8];
};

class FdcanBus {
public:
  explicit FdcanBus(FDCAN_HandleTypeDef* h) : h_(h) {}

  bool initClassic500k(); // filter defaults OFF; you add specific filters
  bool addStdFilter(uint16_t id); // exact match, store in FIFO0
  bool receive(CanFrame& out);    // non-blocking poll FIFO0
  bool send(const CanFrame& in);  // non-blocking

private:
  FDCAN_HandleTypeDef* h_;
};
