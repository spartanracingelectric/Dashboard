#pragma once
#include "stm32h7xx_hal.h"
#include <cstdint>

struct Apa102Color {
    uint8_t r, g, b;
    uint8_t brightness;  // 0-31
};

class Apa102Chain {
public:
    Apa102Chain(GPIO_TypeDef* di_port, uint16_t di_pin,
                GPIO_TypeDef* ci_port, uint16_t ci_pin,
                uint8_t num_leds);

    void setLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b,
                uint8_t brightness = 31);
    void clear();
    void setGlobalBrightness(uint8_t brightness);
    void show();
    uint8_t count() const { return num_leds_; }

private:
    void sendByte(uint8_t byte);
    void sendStartFrame();
    void sendEndFrame();

    inline void diHigh()  { di_port_->BSRR = di_pin_; }
    inline void diLow()   { di_port_->BSRR = (uint32_t)di_pin_ << 16u; }
    inline void ciHigh()  { ci_port_->BSRR = ci_pin_; }
    inline void ciLow()   { ci_port_->BSRR = (uint32_t)ci_pin_ << 16u; }

    GPIO_TypeDef* di_port_;
    uint16_t      di_pin_;
    GPIO_TypeDef* ci_port_;
    uint16_t      ci_pin_;
    uint8_t       num_leds_;
    uint8_t       global_brightness_;

    static constexpr uint8_t MAX_LEDS = 12;
    Apa102Color buffer_[MAX_LEDS];
};
