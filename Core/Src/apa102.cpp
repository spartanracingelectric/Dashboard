#include "apa102.h"

Apa102Chain::Apa102Chain(GPIO_TypeDef* di_port, uint16_t di_pin,
                         GPIO_TypeDef* ci_port, uint16_t ci_pin,
                         uint8_t num_leds)
    : di_port_(di_port), di_pin_(di_pin),
      ci_port_(ci_port), ci_pin_(ci_pin),
      num_leds_(num_leds > MAX_LEDS ? MAX_LEDS : num_leds),
      global_brightness_(31)
{
    clear();
}

void Apa102Chain::clear() {
    for (uint8_t i = 0; i < MAX_LEDS; ++i)
        buffer_[i] = {0, 0, 0, 0};
}

void Apa102Chain::setLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b,
                          uint8_t brightness) {
    if (index >= num_leds_) return;
    buffer_[index] = { r, g, b, (uint8_t)(brightness > 31 ? 31 : brightness) };
}

void Apa102Chain::setGlobalBrightness(uint8_t brightness) {
    global_brightness_ = brightness > 31 ? 31 : brightness;
}

void Apa102Chain::sendByte(uint8_t byte) {
    for (int8_t bit = 7; bit >= 0; --bit) {
        if (byte & (1u << bit))
            diHigh();
        else
            diLow();
        ciHigh();
        ciLow();
    }
}

void Apa102Chain::sendStartFrame() {
    diLow();
    for (uint8_t i = 0; i < 32; ++i) {
        ciHigh();
        ciLow();
    }
}

void Apa102Chain::sendEndFrame() {
    diHigh();
    for (uint8_t i = 0; i < 32; ++i) {
        ciHigh();
        ciLow();
    }
}

void Apa102Chain::show() {
    sendStartFrame();

    for (uint8_t i = 0; i < num_leds_; ++i) {
        const auto& led = buffer_[i];
        uint8_t bright = led.brightness;
        // If explicit brightness is 0 but colors are set, use global brightness
        if (bright == 0 && (led.r || led.g || led.b))
            bright = global_brightness_;
        sendByte(0xE0 | (bright & 0x1F));  // 111 + 5-bit brightness
        sendByte(led.b);                    // Blue first (BGR order)
        sendByte(led.g);                    // Green
        sendByte(led.r);                    // Red last
    }

    sendEndFrame();
}
