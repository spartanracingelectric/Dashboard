// max7219.cpp

#include "max7219.h"
#include "stm32h7xx_hal.h"
#include "main.h"

void Max7219::begin() {
  HAL_GPIO_WritePin(cs_port_, cs_pin_, GPIO_PIN_SET);
  writeReg(0x0C, 0x01); // shutdown=off
  writeReg(0x09, 0x00); // decode=off
  writeReg(0x0B, 0x07); // scan limit=8 digits
  writeReg(0x0F, 0x00); // display test off
  intensity(0x0F);
  for (int r=1;r<=8;++r) writeReg(r, 0x00);
}

void Max7219::intensity(uint8_t v) { writeReg(0x0A, v & 0x0F); }

void Max7219::setPoint(uint8_t row, uint8_t col, bool on) {
  
  if (row>7 || col>7) return;
  uint8_t mask = 1u << col;
  if (on) rows_[row] |= mask; else rows_[row] &= ~mask;
  writeReg(row+1, rows_[row]);
}

void Max7219::writeReg(uint8_t reg, uint8_t data) {
  uint8_t buf[2] = {reg, data};
  HAL_GPIO_WritePin(cs_port_, cs_pin_, GPIO_PIN_RESET);
  HAL_SPI_Transmit(spi_, buf, 2, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(cs_port_, cs_pin_, GPIO_PIN_SET);
}
