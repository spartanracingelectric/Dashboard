// max7219.h
#pragma once
#include "stm32h7xx_hal.h"
#include "config.h"
#include <cstdint>

class Max7219 {
public:
  Max7219(SPI_HandleTypeDef* spi, GPIO_TypeDef* cs_port, uint16_t cs_pin)
  : spi_(spi), cs_port_(cs_port), cs_pin_(cs_pin) {}

  void begin();
  void setPoint(uint8_t row, uint8_t col, bool on); // emulate original "matrix" API  :contentReference[oaicite:44]{index=44}
  void intensity(uint8_t value); // 0..0xF

private:
  void writeReg(uint8_t reg, uint8_t data);
  SPI_HandleTypeDef* spi_;
  GPIO_TypeDef* cs_port_; uint16_t cs_pin_;
  uint8_t rows_[8]{}; // simple 8x8 example; extend if you daisy-chain
};
