// leds.cpp
#include "main.h"
#include "leds.h"
#include <array>
#include <cmath>
#include <algorithm>

static Max7219* g = nullptr;
static uint8_t pin_led_solid_state[NUM_LED_SOLID]{};
static uint32_t prev_overrev=0, prev_revlim=0, prev_hvtemp=0;

// Going to be called by max7219 setpoint function later, needed for that
const uint8_t PIN_LED_SOLID[NUM_LED_SOLID][2] = {{0,0},{0,1},{0,2},{0,3},{0,4},{1,0},{1,1},{1,2},{1,3},{1,4}};  
const uint8_t PIN_LED_RGB_R[NUM_LED_RGB][2]   = {{2,0},{2,2},{2,4},{2,1},{2,3},{2,5}};                            
const uint8_t PIN_LED_RGB_G[NUM_LED_RGB][2]   = {{3,0},{3,2},{3,4},{3,1},{3,3},{3,5}};                             
const uint8_t PIN_LED_RGB_B[NUM_LED_RGB][2]   = {{4,0},{4,2},{4,4},{4,1},{4,3},{4,5}};                            

#if __cplusplus < 201703L
namespace std {
  template <typename T>
  constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
  }
}
#endif

namespace {
  struct EffCfg {
    // Mapping/geometry
    static constexpr int N = NUM_LED_RGB;           // total RGB LEDs
    static constexpr int CENTER_L = (N/2) - 1;      
    static constexpr int CENTER_R = (N/2);          
    // Visual shaping
    float deadband       = 0.02f;   
    float displayRange   = 0.15f;   
    float alphaLPF       = 0.85f;   // error smoothing
    float pulseHz        = 1.0f;    
    float baseBright     = 0.65f;   // full LED brightness (0 to 1)
    float fracBright     = 0.65f;   
    // Dither PWM
    uint16_t pwmHz       = 120;     // software PWM rate
  } cfg;

  //negative = undershoot→right/green, positive = overshoot→left/yellow
  static float g_rawErr = 0.0f;
  static float g_err    = 0.0f;
  static bool  g_hasNew = false;

  // Per-LED desired brightness for each color channel
  static float desR[NUM_LED_RGB]{};
  static float desG[NUM_LED_RGB]{};
  static float desB[NUM_LED_RGB]{};

  // Phases
  static uint32_t g_lastTickMs = 0;
  static float    g_pulsePhase = 0.0f;
  static uint32_t g_lastPwmFlipMs = 0;
  static float    g_pwmPhase01 = 0.0f; 

  inline void clearDesired() {
    for (int i=0;i<NUM_LED_RGB;i++) { desR[i]=desG[i]=desB[i]=0.0f; }
  }

  inline void setRGBDesired(int idx, float r, float g, float b) {
    if (idx < 0 || idx >= NUM_LED_RGB) return;
    desR[idx] = std::clamp(r, 0.0f, 1.0f);
    desG[idx] = std::clamp(g, 0.0f, 1.0f);
    desB[idx] = std::clamp(b, 0.0f, 1.0f);
  }

  inline void hwSetRGB(int i, bool rOn, bool gOn, bool bOn) {
    g->setPoint(PIN_LED_RGB_R[i][0], PIN_LED_RGB_R[i][1], rOn);
    g->setPoint(PIN_LED_RGB_G[i][0], PIN_LED_RGB_G[i][1], gOn);
    g->setPoint(PIN_LED_RGB_B[i][0], PIN_LED_RGB_B[i][1], bOn);
  }

  inline void hwApplyDither() { // 1 bit PWM for ON/OFF for each LED channel
    for (int i=0;i<NUM_LED_RGB;i++) {
      const bool rOn = (desR[i] > g_pwmPhase01);
      const bool gOn = (desG[i] > g_pwmPhase01);
      const bool bOn = (desB[i] > g_pwmPhase01);
      hwSetRGB(i, rOn, gOn, bOn);
    }
  }

  inline void renderOnTarget(float dt_s) {
    clearDesired();
  
    g_pulsePhase += cfg.pulseHz * dt_s;
    const float s = 0.5f + 0.5f * std::sin(2.0f * 3.14159265f * g_pulsePhase);
    const float bright = 0.30f + 0.40f * s; 
    
    setRGBDesired(EffCfg::CENTER_L, 0.85f*bright, 0.92f*bright, 1.00f*bright);
    setRGBDesired(EffCfg::CENTER_R, 0.85f*bright, 0.92f*bright, 1.00f*bright);
  }

  inline void renderBar(float err) {
    clearDesired();
    // Determine side & magnitude
    const float abserr = std::fabs(err);
    if (abserr <= cfg.deadband) return; // on-target handled by caller

    const int L = NUM_LED_RGB / 2; // LEDs per side
    float mag01 = std::clamp(abserr / cfg.displayRange, 0.0f, 1.0f);
    const float pos = mag01 * float(L);      // 0..L
    const int   k   = std::min(int(std::floor(pos)), L); // full LEDs
    const float f   = std::clamp(pos - float(k), 0.0f, 1.0f);

    const bool overshoot = (err > 0.0f); // overshoot → left (yellow), undershoot → right (green)
    const float fullB = cfg.baseBright;
    const float fracB = cfg.fracBright;

    auto setGreen = [&](int idx, float b){ setRGBDesired(idx, 0.0f, b, 0.0f); };
    auto setYellow=[&](int idx, float b){ setRGBDesired(idx, b,   b, 0.0f); };

    // Anchor: faint center pair to stabilize eye
    setRGBDesired(EffCfg::CENTER_L, 0.06f, 0.06f, 0.06f);
    setRGBDesired(EffCfg::CENTER_R, 0.06f, 0.06f, 0.06f);

    if (!overshoot) {
      // undershoot → rightwards from CENTER_R
      // Full LEDs
      for (int i=0;i<k; ++i) {
        int idx = EffCfg::CENTER_R + i;
        if (idx >= 0 && idx < NUM_LED_RGB) setGreen(idx, fullB);
      }
      // Fractional
      if (k < L && f > 1e-3f) {
        int idxF = EffCfg::CENTER_R + k;
        if (idxF >=0 && idxF < NUM_LED_RGB) setGreen(idxF, fracB * f);
      }
    } else {
      // overshoot → leftwards from CENTER_L
      for (int i=0;i<k; ++i) {
        int idx = EffCfg::CENTER_L - i;
        if (idx >= 0 && idx < NUM_LED_RGB) setYellow(idx, fullB);
      }
      if (k < L && f > 1e-3f) {
        int idxF = EffCfg::CENTER_L - k;
        if (idxF >=0 && idxF < NUM_LED_RGB) setYellow(idxF, fracB * f);
      }
    }
  }
}

namespace leds {

void init(Max7219* dev) { g = dev; g->begin(); }

/*
void wake() {
  // Simple chase like original  
  for (int i=0;i<NUM_LED_RGB;i++) { g->setPoint(PIN_LED_RGB_R[i][0], PIN_LED_RGB_R[i][1], true); HAL_Delay(50); }
  for (int i=0;i<NUM_LED_RGB;i++) { g->setPoint(PIN_LED_RGB_R[i][0], PIN_LED_RGB_R[i][1], false); HAL_Delay(50); }
  for (uint8_t i=0;i<NUM_LED_SOLID;i++) g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], true);
  HAL_Delay(100);
  for (uint8_t i=0;i<NUM_LED_SOLID;i++) g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], false);
}
*/

//Updated Wake function, uncomment top one if this does not work
void wake() {
  // Per-LED: turn on R -> G -> B (like leds__wake)
  for (int i = 0; i < NUM_LED_RGB; i++) {
    g->setPoint(PIN_LED_RGB_R[i][0], PIN_LED_RGB_R[i][1], true);
    HAL_Delay(50);
    g->setPoint(PIN_LED_RGB_G[i][0], PIN_LED_RGB_G[i][1], true);
    HAL_Delay(50);
    g->setPoint(PIN_LED_RGB_B[i][0], PIN_LED_RGB_B[i][1], true);
    HAL_Delay(50);
  }

  // Per-LED: turn off R -> G -> B
  for (int i = 0; i < NUM_LED_RGB; i++) {
    g->setPoint(PIN_LED_RGB_R[i][0], PIN_LED_RGB_R[i][1], false);
    HAL_Delay(50);
    g->setPoint(PIN_LED_RGB_G[i][0], PIN_LED_RGB_G[i][1], false);
    HAL_Delay(50);
    g->setPoint(PIN_LED_RGB_B[i][0], PIN_LED_RGB_B[i][1], false);
    HAL_Delay(50);
  }

  // Solid LEDs: on -> off -> on -> off (two blinks), 100 ms cadence
  for (uint8_t i = 0; i < NUM_LED_SOLID; i++)
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], true);
  HAL_Delay(100);

  for (uint8_t i = 0; i < NUM_LED_SOLID; i++)
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], false);
  HAL_Delay(100);

  for (uint8_t i = 0; i < NUM_LED_SOLID; i++)
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], true);
  HAL_Delay(100);

  for (uint8_t i = 0; i < NUM_LED_SOLID; i++)
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], false);
}

void efficiency_on_can_ratio(float ratio) {
  // Signed error where +ve means overshoot (lay off), -ve means undershoot (push)
  ratio = std::clamp(ratio, 0.0f, 4.0f); 
  g_rawErr = ratio - 1.0f;
  g_hasNew = true;
}


void efficiency_on_can_error(float signed_err) {
  signed_err = std::clamp(signed_err, -2.0f, 2.0f);
  g_rawErr = signed_err;
  g_hasNew = true;
}


void efficiency_tick(uint32_t now_ms) {
  if (!g) return;

  // Timebase
  if (g_lastTickMs == 0) g_lastTickMs = now_ms;
  const uint32_t dt_ms = now_ms - g_lastTickMs;
  g_lastTickMs = now_ms;
  const float dt_s = dt_ms * 0.001f;

  // Smooth error on new samples
  if (g_hasNew) {
    g_err = cfg.alphaLPF * g_err + (1.0f - cfg.alphaLPF) * g_rawErr;
    g_hasNew = false;
  }


  if (std::fabs(g_err) <= cfg.deadband) {
    renderOnTarget(dt_s);
  } else {
    renderBar(g_err);
  }

 
  const uint32_t pwmPeriodMs = (cfg.pwmHz == 0) ? 10 : (1000u / cfg.pwmHz);
  if (pwmPeriodMs == 0) {
    g_pwmPhase01 = 0.0f;
  } else {
    
    uint32_t elapsed = now_ms - g_lastPwmFlipMs;
    if (elapsed >= pwmPeriodMs) {
      
      g_lastPwmFlipMs = now_ms - (elapsed % pwmPeriodMs);
      elapsed = elapsed % pwmPeriodMs;
    }
    g_pwmPhase01 = float(elapsed) / float(pwmPeriodMs);
  }

 
  hwApplyDither();
}


void enable_shift() {
  for (uint8_t i=0;i<NUM_LED_SOLID-2;i++) {
    pin_led_solid_state[i+1] = 1;
    g->setPoint(PIN_LED_SOLID[i+1][0], PIN_LED_SOLID[i+1][1], true);
  }
  if (pin_led_solid_state[1]) {
    pin_led_solid_state[0]=0; pin_led_solid_state[NUM_LED_SOLID-1]=0;
    g->setPoint(PIN_LED_SOLID[0][0], PIN_LED_SOLID[0][1], false);
    g->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-1][0], PIN_LED_SOLID[NUM_LED_SOLID-1][1], false);
  }
}
void disable_shift() {
  for (uint8_t i=0;i<NUM_LED_SOLID-2;i++) {
    pin_led_solid_state[i+1] = 0;
    g->setPoint(PIN_LED_SOLID[i+1][0], PIN_LED_SOLID[i+1][1], false);
  }
}
void disable_all_solid() {
  for (uint8_t i=0;i<NUM_LED_SOLID;i++) {
    pin_led_solid_state[i]=0;
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], false);
  }
}
void disable_half_solid(bool firstHalf) {
  uint8_t s = firstHalf ? 0 : NUM_LED_SOLID/2;
  uint8_t e = firstHalf ? NUM_LED_SOLID/2 : NUM_LED_SOLID;
  for (uint8_t i=s;i<e;i++) {
    pin_led_solid_state[i]=0;
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], false);
  }
}
void toggle_overrev() {
  for (uint8_t i=1;i<NUM_LED_SOLID-1;i++) {
    bool on = !pin_led_solid_state[i];
    pin_led_solid_state[i]= on;
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], on);
  }
  if (pin_led_solid_state[1]) {
    pin_led_solid_state[0]=0; pin_led_solid_state[NUM_LED_SOLID-1]=0;
    g->setPoint(PIN_LED_SOLID[0][0], PIN_LED_SOLID[0][1], false);
    g->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-1][0], PIN_LED_SOLID[NUM_LED_SOLID-1][1], false);
  }
}
void toggle_half(bool firstHalf) {
  uint8_t s = firstHalf ? 0 : NUM_LED_SOLID/2;
  uint8_t e = firstHalf ? NUM_LED_SOLID/2 : NUM_LED_SOLID;
  for (uint8_t i=s;i<e;i++) {
    bool on = !pin_led_solid_state[i];
    pin_led_solid_state[i]=on;
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], on);
  }
}
void toggle_revlim() {
  for (uint8_t i=1;i<NUM_LED_SOLID-1;i++) {
    bool on = !pin_led_solid_state[i];
    pin_led_solid_state[i]=on;
    g->setPoint(PIN_LED_SOLID[i][0], PIN_LED_SOLID[i][1], on);
  }
  if (pin_led_solid_state[1]) {
    pin_led_solid_state[0]=0; pin_led_solid_state[NUM_LED_SOLID-1]=0;
    g->setPoint(PIN_LED_SOLID[0][0], PIN_LED_SOLID[0][1], false);
    g->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-1][0], PIN_LED_SOLID[NUM_LED_SOLID-1][1], false);
  } else {
    pin_led_solid_state[0]=1; pin_led_solid_state[NUM_LED_SOLID-1]=1;
    g->setPoint(PIN_LED_SOLID[0][0], PIN_LED_SOLID[0][1], true);
    g->setPoint(PIN_LED_SOLID[NUM_LED_SOLID-1][0], PIN_LED_SOLID[NUM_LED_SOLID-1][1], true);
  }
}
void set_brightness(uint8_t v) { g->intensity(v); }

void lv(float lv) {
  bool low = (lv < LV_WARNING_THRESHOLD);
 
  g->setPoint(3,1,false);
  g->setPoint(PIN_LED_RGB_R[3][0], PIN_LED_RGB_R[3][1], low);
}

void hvtemp(float hvtemp) {
  bool hot = (hvtemp > HVTEMP_LIMIT_C);
  g->setPoint(3,1,false);
  g->setPoint(PIN_LED_RGB_R[3][0], PIN_LED_RGB_R[3][1], hot);
}

void safety_update_flash(float hvtemp, uint32_t now_ms) {
  if (hvtemp > HVTEMP_LIMIT_C) { 
    static uint32_t last = 0;
    if (now_ms - last >= HVTEMP_THRESHOLD_FLASH_MS) {
      last = now_ms;
      toggle_half(false); // second half
    }
  } else {
    disable_half_solid(false);
  }
}
void efficiency_on_can_ratio(float ratio);
void efficiency_on_can_error(float signed_err);

void efficiency_tick(uint32_t now_ms);
} // namespace leds
