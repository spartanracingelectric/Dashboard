// leds.cpp
#include "main.h"
#include "leds.h"
#include "apa102.h"
#include <cmath>
#include <algorithm>

static Apa102Chain* g_bar   = nullptr;
static Apa102Chain* g_left  = nullptr;
static Apa102Chain* g_right = nullptr;

// Shift indicator state on bar chain
static uint8_t bar_solid_state[NUM_LED_BAR]{};
static bool g_shiftActive = false;

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
    static constexpr int N = NUM_LED_BAR;          // 12 LEDs
    static constexpr int CENTER_L = (N/2) - 1;     // index 5
    static constexpr int CENTER_R = (N/2);          // index 6
    float deadband       = 0.02f;
    float displayRange   = 0.15f;
    float alphaLPF       = 0.85f;
    float pulseHz        = 1.0f;
    float baseBright     = 0.65f;
    float fracBright     = 0.65f;
  } cfg;

  // negative = undershoot→right/green, positive = overshoot→left/yellow
  static float g_rawErr = 0.0f;
  static float g_err    = 0.0f;
  static bool  g_hasNew = false;

  // Per-bar-LED desired brightness (0.0–1.0)
  static float desR[NUM_LED_BAR]{};
  static float desG[NUM_LED_BAR]{};
  static float desB[NUM_LED_BAR]{};

  // Left chain warning state
  static bool  g_leftOverride[NUM_LED_LEFT]{};
  static float g_leftR[NUM_LED_LEFT]{}, g_leftG[NUM_LED_LEFT]{}, g_leftB[NUM_LED_LEFT]{};

  // Right chain warning state
  static bool  g_rightOverride[NUM_LED_RIGHT]{};
  static float g_rightR[NUM_LED_RIGHT]{}, g_rightG[NUM_LED_RIGHT]{}, g_rightB[NUM_LED_RIGHT]{};

  // Timebase
  static uint32_t g_lastTickMs = 0;
  static float    g_pulsePhase = 0.0f;

  inline void clearDesired() {
    for (int i = 0; i < NUM_LED_BAR; i++) { desR[i] = desG[i] = desB[i] = 0.0f; }
  }

  inline void setRGBDesired(int idx, float r, float g, float b) {
    if (idx < 0 || idx >= NUM_LED_BAR) return;
    desR[idx] = std::clamp(r, 0.0f, 1.0f);
    desG[idx] = std::clamp(g, 0.0f, 1.0f);
    desB[idx] = std::clamp(b, 0.0f, 1.0f);
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
    const float abserr = std::fabs(err);
    if (abserr <= cfg.deadband) return;

    const int L = NUM_LED_BAR / 2;  // 6 LEDs per side
    float mag01 = std::clamp(abserr / cfg.displayRange, 0.0f, 1.0f);
    const float pos = mag01 * float(L);
    const int   k   = std::min(int(std::floor(pos)), L);
    const float f   = std::clamp(pos - float(k), 0.0f, 1.0f);

    const bool overshoot = (err > 0.0f);
    const float fullB = cfg.baseBright;
    const float fracB = cfg.fracBright;

    auto setGreen  = [&](int idx, float b){ setRGBDesired(idx, 0.0f, b, 0.0f); };
    auto setYellow = [&](int idx, float b){ setRGBDesired(idx, b,    b, 0.0f); };

    // Faint center anchor
    setRGBDesired(EffCfg::CENTER_L, 0.06f, 0.06f, 0.06f);
    setRGBDesired(EffCfg::CENTER_R, 0.06f, 0.06f, 0.06f);

    if (!overshoot) {
      for (int i = 0; i < k; ++i) {
        int idx = EffCfg::CENTER_R + i;
        if (idx >= 0 && idx < NUM_LED_BAR) setGreen(idx, fullB);
      }
      if (k < L && f > 1e-3f) {
        int idxF = EffCfg::CENTER_R + k;
        if (idxF >= 0 && idxF < NUM_LED_BAR) setGreen(idxF, fracB * f);
      }
    } else {
      for (int i = 0; i < k; ++i) {
        int idx = EffCfg::CENTER_L - i;
        if (idx >= 0 && idx < NUM_LED_BAR) setYellow(idx, fullB);
      }
      if (k < L && f > 1e-3f) {
        int idxF = EffCfg::CENTER_L - k;
        if (idxF >= 0 && idxF < NUM_LED_BAR) setYellow(idxF, fracB * f);
      }
    }
  }

  // Flush shift indicator pattern to bar chain (white LEDs for active positions)
  inline void applyBarSolid() {
    if (!g_bar) return;
    for (uint8_t i = 0; i < NUM_LED_BAR; ++i) {
      if (bar_solid_state[i])
        g_bar->setLed(i, 255, 255, 255, 31);
      else
        g_bar->setLed(i, 0, 0, 0, 0);
    }
    g_bar->show();
  }
}

namespace leds {

void init(Apa102Chain* bar, Apa102Chain* left, Apa102Chain* right) {
    g_bar = bar;
    g_left = left;
    g_right = right;
    g_bar->clear();  g_bar->show();
    g_left->clear(); g_left->show();
    g_right->clear();g_right->show();
}

void wake() {
  // Chase red across bar
  for (int i = 0; i < NUM_LED_BAR; ++i) {
    g_bar->setLed(i, 255, 0, 0, 31);
    g_bar->show();
    HAL_Delay(50);
  }
  g_bar->clear();
  g_bar->show();

  // Flash left and right green
  for (int i = 0; i < NUM_LED_LEFT; ++i)
    g_left->setLed(i, 0, 255, 0, 31);
  for (int i = 0; i < NUM_LED_RIGHT; ++i)
    g_right->setLed(i, 0, 255, 0, 31);
  g_left->show();
  g_right->show();
  HAL_Delay(200);

  g_left->clear();
  g_left->show();
  g_right->clear();
  g_right->show();
}

void efficiency_on_can_ratio(float ratio) {
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
  if (!g_bar) return;

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

  // Render efficiency bar (skip if shift indicator is active)
  if (!g_shiftActive) {
    if (std::fabs(g_err) <= cfg.deadband) {
      renderOnTarget(dt_s);
    } else {
      renderBar(g_err);
    }

    // Flush bar chain: convert float 0–1 → uint8 0–255
    for (int i = 0; i < NUM_LED_BAR; ++i) {
      g_bar->setLed(i,
        (uint8_t)(desR[i] * 255.0f),
        (uint8_t)(desG[i] * 255.0f),
        (uint8_t)(desB[i] * 255.0f),
        31);
    }
    g_bar->show();
  }

  // Flush left chain warnings
  for (int i = 0; i < NUM_LED_LEFT; ++i) {
    if (g_leftOverride[i])
      g_left->setLed(i,
        (uint8_t)(g_leftR[i] * 255.0f),
        (uint8_t)(g_leftG[i] * 255.0f),
        (uint8_t)(g_leftB[i] * 255.0f), 31);
    else
      g_left->setLed(i, 0, 0, 0, 0);
  }
  g_left->show();

  // Flush right chain warnings
  for (int i = 0; i < NUM_LED_RIGHT; ++i) {
    if (g_rightOverride[i])
      g_right->setLed(i,
        (uint8_t)(g_rightR[i] * 255.0f),
        (uint8_t)(g_rightG[i] * 255.0f),
        (uint8_t)(g_rightB[i] * 255.0f), 31);
    else
      g_right->setLed(i, 0, 0, 0, 0);
  }
  g_right->show();
}

// --- Shift indicator functions (bar chain, white LEDs) ---

void enable_shift() {
  g_shiftActive = true;
  for (uint8_t i = 1; i <= NUM_LED_BAR - 2; ++i)
    bar_solid_state[i] = 1;
  bar_solid_state[0] = 0;
  bar_solid_state[NUM_LED_BAR - 1] = 0;
  applyBarSolid();
}

void disable_shift() {
  for (uint8_t i = 1; i <= NUM_LED_BAR - 2; ++i)
    bar_solid_state[i] = 0;
  g_shiftActive = false;
  applyBarSolid();
}

void disable_all_solid() {
  for (uint8_t i = 0; i < NUM_LED_BAR; ++i)
    bar_solid_state[i] = 0;
  g_shiftActive = false;
  applyBarSolid();
}

void disable_half_solid(bool firstHalf) {
  uint8_t s = firstHalf ? 0 : NUM_LED_BAR / 2;
  uint8_t e = firstHalf ? NUM_LED_BAR / 2 : NUM_LED_BAR;
  for (uint8_t i = s; i < e; ++i)
    bar_solid_state[i] = 0;
  // Check if any still active
  g_shiftActive = false;
  for (uint8_t i = 0; i < NUM_LED_BAR; ++i)
    if (bar_solid_state[i]) { g_shiftActive = true; break; }
  applyBarSolid();
}

void toggle_overrev() {
  g_shiftActive = true;
  for (uint8_t i = 1; i < NUM_LED_BAR - 1; ++i)
    bar_solid_state[i] = !bar_solid_state[i];
  if (bar_solid_state[1]) {
    bar_solid_state[0] = 0;
    bar_solid_state[NUM_LED_BAR - 1] = 0;
  }
  applyBarSolid();
}

void toggle_half(bool firstHalf) {
  g_shiftActive = true;
  uint8_t s = firstHalf ? 0 : NUM_LED_BAR / 2;
  uint8_t e = firstHalf ? NUM_LED_BAR / 2 : NUM_LED_BAR;
  for (uint8_t i = s; i < e; ++i)
    bar_solid_state[i] = !bar_solid_state[i];
  applyBarSolid();
}

void toggle_revlim() {
  g_shiftActive = true;
  for (uint8_t i = 1; i < NUM_LED_BAR - 1; ++i)
    bar_solid_state[i] = !bar_solid_state[i];
  if (bar_solid_state[1]) {
    bar_solid_state[0] = 0;
    bar_solid_state[NUM_LED_BAR - 1] = 0;
  } else {
    bar_solid_state[0] = 1;
    bar_solid_state[NUM_LED_BAR - 1] = 1;
  }
  applyBarSolid();
}

void set_brightness(uint8_t v) {
  if (v > 31) v = 31;
  if (g_bar)   g_bar->setGlobalBrightness(v);
  if (g_left)  g_left->setGlobalBrightness(v);
  if (g_right) g_right->setGlobalBrightness(v);
}

// --- Safety warnings (left/right chains) ---

void lv(float lv) {
  bool low = (lv < LV_WARNING_THRESHOLD);
  g_leftOverride[0] = low;
  g_leftR[0] = low ? 1.0f : 0.0f;
  g_leftG[0] = 0.0f;
  g_leftB[0] = 0.0f;
}

void hvtemp(float hvtemp) {
  bool hot = (hvtemp > HVTEMP_LIMIT_C);
  g_leftOverride[1] = hot;
  g_leftR[1] = hot ? 1.0f : 0.0f;
  g_leftG[1] = 0.0f;
  g_leftB[1] = 0.0f;
}

void safety_update_flash(float hvtemp, uint32_t now_ms) {
  if (hvtemp > HVTEMP_LIMIT_C) {
    static uint32_t last = 0;
    if (now_ms - last >= HVTEMP_THRESHOLD_FLASH_MS) {
      last = now_ms;
      static bool on = false;
      on = !on;
      g_rightOverride[0] = true;
      g_rightR[0] = on ? 1.0f : 0.0f;
      g_rightG[0] = 0.0f;
      g_rightB[0] = 0.0f;
    }
  } else {
    g_rightOverride[0] = false;
  }
}

void led0_on()
{
	g_bar->setLed(0, 255, 0, 0, 31); // LED0 = red
	g_bar->show();
}

void led0_off()
{
	g_bar->setLed(0, 0, 0, 0, 0); // LED0 = red
	g_bar->show();
}

} // namespace leds
