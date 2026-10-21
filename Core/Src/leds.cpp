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
  static bool  g_everReceived = false;

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

  // On budget: gentle white pulse on first LED so driver knows system is alive
  inline void renderOnTarget(float dt_s) {
    clearDesired();
    g_pulsePhase += cfg.pulseHz * dt_s;
    const float s = 0.5f + 0.5f * std::sin(2.0f * 3.14159265f * g_pulsePhase);
    const float bright = 0.20f + 0.30f * s;
    setRGBDesired(0, bright, bright, bright);  // white pulse on LED 0
  }

  // Fill left-to-right based on magnitude of error:
  //   Green  = under budget (negative error, saving energy)
  //   Yellow = over budget  (positive error, using too much)
  inline void renderBar(float err) {
    clearDesired();
    const float abserr = std::fabs(err);

    // Map error magnitude to number of LEDs (0 → 0, displayRange → all 12)
    float mag01 = std::clamp(abserr / cfg.displayRange, 0.0f, 1.0f);
    const float pos = mag01 * (float)NUM_LED_BAR;
    const int   k   = std::min((int)std::floor(pos), (int)NUM_LED_BAR);
    const float f   = std::clamp(pos - (float)k, 0.0f, 1.0f);

    const float fullB = cfg.baseBright;
    const float fracB = cfg.fracBright;

    for (int i = 0; i < k; ++i) {
      if (err > 0.0f)
        setRGBDesired(i, fullB, fullB, 0.0f);   // yellow = over budget
      else
        setRGBDesired(i, 0.0f, fullB, 0.0f);    // green  = under budget
    }
    // Fractional LED at the tip
    if (k < NUM_LED_BAR && f > 1e-3f) {
      if (err > 0.0f)
        setRGBDesired(k, fracB * f, fracB * f, 0.0f);
      else
        setRGBDesired(k, 0.0f, fracB * f, 0.0f);
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

void enable_all()
{
	g_bar->clear();
	g_left->clear();
	g_right->clear();

	for (int i = 0; i < NUM_LED_BAR; ++i) {
	    g_bar->setLed(i, 255, 0, 0, 2);
	}
	for (int i = 0; i < NUM_LED_LEFT; ++i) {
	    g_left->setLed(i, 0, 255, 0, 2);
	}
	for (int i = 0; i < NUM_LED_RIGHT; ++i) {
		g_right->setLed(i, 0, 255, 0, 2);
	}

	g_bar->show();
	g_left->show();
	g_right->show();
}

void efficiency_on_can_ratio(float ratio) {
  ratio = std::clamp(ratio, 0.0f, 4.0f);
  g_rawErr = ratio - 1.0f;
  g_hasNew = true;
  g_everReceived = true;
}

void efficiency_on_can_error(float signed_err) {
  signed_err = std::clamp(signed_err, -2.0f, 2.0f);
  g_rawErr = signed_err;
  g_hasNew = true;
  g_everReceived = true;
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
    if (!g_everReceived) {
      clearDesired();
    } else if (std::fabs(g_err) <= cfg.deadband) {
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

// --- Left chain: highest cell temperature bar (no red) ---
// Progressive fill: 0 LEDs below CELLTEMP_LO, all 3 LEDs at/above CELLTEMP_HI.
// LED 0 green, LED 1 green-yellow, LED 2 yellow — warmer hues as more LEDs light.
void celltemp(float temp_c) {
  const float lo = 25.0f, hi = 55.0f;
  float n = (temp_c - lo) / (hi - lo);
  if (n < 0.0f) n = 0.0f;
  if (n > 1.0f) n = 1.0f;
  int lit = (int)(n * (float)NUM_LED_LEFT + 0.5f);
  if (lit > NUM_LED_LEFT) lit = NUM_LED_LEFT;

  for (int i = 0; i < NUM_LED_LEFT; i++) {
    if (i < lit) {
      g_leftOverride[i] = true;
      if (i == 0)      { g_leftR[i] = 0.0f; g_leftG[i] = 1.0f; g_leftB[i] = 0.0f; }
      else if (i == 1) { g_leftR[i] = 0.8f; g_leftG[i] = 1.0f; g_leftB[i] = 0.0f; }
      else             { g_leftR[i] = 1.0f; g_leftG[i] = 1.0f; g_leftB[i] = 0.0f; }
    } else {
      g_leftOverride[i] = false;
    }
  }
}

// --- Right chain: state-of-charge bar (no red) ---
// Progressive fill in green; number lit scales with SoC percentage.
void soc(float soc_pct) {
  if (soc_pct < 0.0f) soc_pct = 0.0f;
  if (soc_pct > 100.0f) soc_pct = 100.0f;
  int lit = (int)((soc_pct / 100.0f) * (float)NUM_LED_RIGHT + 0.5f);
  if (lit > NUM_LED_RIGHT) lit = NUM_LED_RIGHT;

  for (int i = 0; i < NUM_LED_RIGHT; i++) {
    if (i < lit) {
      g_rightOverride[i] = true;
      g_rightR[i] = 0.0f;
      g_rightG[i] = 1.0f;
      g_rightB[i] = 0.0f;
    } else {
      g_rightOverride[i] = false;
    }
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
