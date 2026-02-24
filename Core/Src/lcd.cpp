// LCD Updated Code with Energy bar (not sure if working or not)
#include "main.h"
#include <cstdio>
#include <cstdint>
#include <cmath>
#include "config.h"
#include "lcd.h"
#include "leds.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include "CFA10100_defines.h"

static uint32_t s_FWol = 0;
static uint8_t  s_bg_r=0, s_bg_g=0, s_bg_b=0;
static uint8_t  s_fg_r=255, s_fg_g=255, s_fg_b=255;

static inline void eve_flush() { EVE_REG_Write_16(EVE_REG_CMD_WRITE, s_FWol); }
static inline void eve_wait()  { s_FWol = Wait_for_EVE_Execution_Complete(s_FWol); }


// EVE HAL stubs, need to be replaced with ioc file actual functions from Carlie
extern "C" {
  void EVE_Init(){
    s_FWol = 0;
    s_bg_r = s_bg_g = s_bg_b = 0;
    s_fg_r = s_fg_g = s_fg_b = 255;
  }

  void EVE_BeginFrame(){
	s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_CMD_DLSTART);
	s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_CLEAR_COLOR_RGB(s_bg_r, s_bg_g, s_bg_b));
	s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_CLEAR(1,1,1));
	s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_COLOR_RGB(s_fg_r, s_fg_g, s_fg_b));

  }

  void EVE_EndFrame(){

    s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_DISPLAY());
    s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_CMD_SWAP);
    eve_flush();
    //eve_wait();    // optional to keep UI synched
  }

  void EVE_SetBackColor(uint8_t r,uint8_t g,uint8_t b){

    s_bg_r=r; s_bg_g=g; s_bg_b=b;
  }


  void EVE_SetFgColor(uint8_t r,uint8_t g,uint8_t b){

    s_fg_r=r; s_fg_g=g; s_fg_b=b;
  }
  void EVE_Text(int16_t x,int16_t y,uint8_t font,uint16_t options,const char* s){

    s_FWol = ::EVE_Text(s_FWol, x, y, font, options, s);

  }
  void EVE_RectFill(int16_t x0,int16_t y0,int16_t x1,int16_t y1){

	s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_BEGIN(EVE_BEGIN_RECTS));
    s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_VERTEX2F(x0*16, y0*16));
    s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_VERTEX2F(x1*16, y1*16));
    s_FWol = EVE_Cmd_Dat_0(s_FWol, EVE_ENC_END());
  }
}

static constexpr float ENERGY_LIMIT_KWH   = 6.50f;   // hard cap
static constexpr float ENERGY_BUFFER_KWH  = 0.10f;   // buffer
static constexpr float UI_SMOOTH_TAU_S    = 0.50f;   
static constexpr uint32_t UI_BLINK_FAST_MS= 200;

// Can change this line below once we know what can frame will have energy consumption
__attribute__((weak)) float can_service_get_energy_used_kWh() { return 0.0f; } // will be provided from CAN so can remove if no errors on removing

// ==== Screen geometry ====
static constexpr int16_t TFT_W = 800;
static constexpr int16_t TFT_H = 480;

static constexpr uint8_t FONT_LABEL = 26;
static constexpr uint8_t FONT_MED   = 28;
static constexpr uint8_t FONT_BIG   = 30;

namespace layout {
  // Labels
  static constexpr int16_t LBL_HVT_X    = 800-120;
  static constexpr int16_t LBL_HVT_Y    = 480-60;
  static constexpr int16_t LBL_TPS0_X   = 24;
  static constexpr int16_t LBL_TPS0_Y   = 480-60;
  static constexpr int16_t LBL_VNLOAD_X = 320;
  static constexpr int16_t LBL_VNLOAD_Y = 240-20;
  static constexpr int16_t LBL_TPS1_X   = 330;
  static constexpr int16_t LBL_TPS1_Y   = 480-100;

  // Values
  static constexpr int16_t HV_X         = 320;
  static constexpr int16_t HV_Y         = 90;
  static constexpr int16_t TPS0_X       = 24;
  static constexpr int16_t TPS0_Y       = 480-20;
  static constexpr int16_t TPS1_X       = 330;
  static constexpr int16_t TPS1_Y       = 480-60;
  static constexpr int16_t HVTEMP_X     = 800-140;
  static constexpr int16_t HVTEMP_Y     = 480-20;

  static constexpr int16_t VNLOAD_X     = 320;
  static constexpr int16_t VNLOAD_Y     = 240+24;

  // Diagnostics
  static constexpr int16_t DIAG_CF_X    = 350;
  static constexpr int16_t DIAG_CF_Y    = 70;
  static constexpr int16_t DIAG_BMS_X   = 350;
  static constexpr int16_t DIAG_BMS_Y   = 158;

  static constexpr int16_t RPM_X        = HV_X;
  static constexpr int16_t RPM_Y        = HV_Y;

  // Sections for clear_section()
  static constexpr int16_t SECT_HV[4]      = { 200,  30, 440, 140 };
  static constexpr int16_t SECT_TPS0[4]    = {   0, 420, 220, 480 };
  static constexpr int16_t SECT_TPS1[4]    = { 260, 400, 460, 480 };
  static constexpr int16_t SECT_HVTEMP[4]  = { 640, 420, 800, 480 };
  static constexpr int16_t SECT_VNLOAD[4]  = { 250, 200, 520, 300 };
}

//Value caches
static float    hv_prev      = -1.f;
static float    tps0_prev    = -1.f;
static float    tps1_prev    = -1.f;
static float    hvtemp_prev  = -1.f;
static float    hvlow_prev   = -1.f;
static uint16_t rpm_prev     = 0xFFFF;

static uint32_t prev_tick_ms = 0;

// Cached formatted strings
static char s_hv[16]      = "";
static char s_tps0[16]    = "";
static char s_tps1[16]    = "";
static char s_hvtemp[16]  = "";
static char s_vnload[16]  = "";
static char s_diag_cf[8]  = "";
static char s_diag_bms[8] = "";
static char s_rpm[16]     = "";

static inline bool changed_f(float a, float b, float eps) {
  return std::fabs(a - b) >= eps;
}

namespace {

void draw_static_labels() {
  EVE_Text(layout::LBL_HVT_X,    layout::LBL_HVT_Y,    FONT_LABEL, 0, "HV T");
  EVE_Text(layout::LBL_TPS0_X,   layout::LBL_TPS0_Y,   FONT_LABEL, 0, "TPS0 %");
  EVE_Text(layout::LBL_VNLOAD_X, layout::LBL_VNLOAD_Y, FONT_LABEL, 0, "No Load Voltage");
  EVE_Text(layout::LBL_TPS1_X,   layout::LBL_TPS1_Y,   FONT_LABEL, 0, "TPS1%");
}

void draw_dynamic_values() {
  EVE_Text(layout::HV_X,      layout::HV_Y,      FONT_BIG, 0, s_hv);
  EVE_Text(layout::TPS0_X,    layout::TPS0_Y,    FONT_MED, 0, s_tps0);
  EVE_Text(layout::TPS1_X,    layout::TPS1_Y,    FONT_MED, 0, s_tps1);
  EVE_Text(layout::HVTEMP_X,  layout::HVTEMP_Y,  FONT_MED, 0, s_hvtemp);
  EVE_Text(layout::VNLOAD_X,  layout::VNLOAD_Y,  FONT_MED, 0, s_vnload);
}

void render_base_screen() {
  EVE_BeginFrame();
  draw_static_labels();
  draw_dynamic_values();

  // Option B to show energy bar on screen
  // energybar::draw(prev_tick_ms);

  EVE_EndFrame();
}

}

// ============================== Energy Bar (EVE) =============================
namespace energybar {

  // Layout (centered)
  static constexpr int BAR_W = 560;
  static constexpr int BAR_H = 90;
  static constexpr int BAR_X = (TFT_W - BAR_W) / 2;
  static constexpr int BAR_Y = (TFT_H - BAR_H) / 2;

  static constexpr int SEG_N   = 5;
  static constexpr int SEG_GAP = 8;

  struct RGB { uint8_t r,g,b; };
  static constexpr RGB SEG_COLORS[SEG_N] = {
    {  28,168, 28},  // green
    { 140,180, 20},  // yellow-green
    { 220,180, 30},  // amber
    { 230,120, 30},  // orange
    { 210, 40, 30},  // red
  };

  // Smoothing state
  static float    s_disp_frac = 0.0f;   // displayed fraction (0..1+)
  static uint32_t s_last_ms   = 0;

  // helpers
  static inline float clampf(float v, float lo, float hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
  }

  static float smooth_step(float cur, float target, float dt_s, float tau_s) {
    if (tau_s <= 0.f) return target;
    float a = 1.f - std::exp(-dt_s / tau_s);
    if (a < 0.f) a = 0.f;
    if (a > 1.f) a = 1.f;
    return cur + a * (target - cur);
  }

  static inline void rect(int x, int y, int w, int h) {
    // EVE_RectFill expects absolute corners
    EVE_RectFill((int16_t)x, (int16_t)y, (int16_t)(x+w), (int16_t)(y+h));
  }

  static int fillUnitsToX(int units, int segW) {
    int x = BAR_X + 2;
    for (int i = 0; i < SEG_N; ++i) {
      if (units <= segW) return x + units;
      units -= segW;
      x += segW + SEG_GAP;
    }
    return BAR_X + BAR_W - 2;
  }

  /*
  void init(uint32_t now_ms) {
    s_disp_frac = 0.0f;
    s_last_ms   = now_ms;
  }
  */

  // Update
  void update(uint32_t now_ms) {
    if (s_last_ms == 0) s_last_ms = now_ms;
    const float used_kWh = can_service_get_energy_used_kWh();
    const float target   = clampf(used_kWh / ENERGY_LIMIT_KWH, 0.f, 1.10f); // show up to +10%
    const float dt       = (now_ms - s_last_ms) / 1000.0f;
    s_disp_frac = smooth_step(s_disp_frac, target, dt, UI_SMOOTH_TAU_S);
    s_last_ms   = now_ms;
  }

  // Draw only the bar (center of screen)
  void draw(uint32_t now_ms) {
    // Background panel (grey)
    EVE_SetFgColor(34, 34, 34);     rect(BAR_X-4, BAR_Y-4, BAR_W+8, BAR_H+8); // frame
    EVE_SetFgColor(245,245,245);    rect(BAR_X+0, BAR_Y+0, BAR_W,   BAR_H);    // fill

    const int segW          = (BAR_W - (SEG_GAP * (SEG_N - 1))) / SEG_N;
    const int totalFillable = segW * SEG_N;

    // Unfilled segment shells
    int x = BAR_X + 2;
    const int segH = BAR_H - 4;
    for (int i = 0; i < SEG_N; ++i) {
      EVE_SetFgColor(220,220,220);
      rect(x, BAR_Y + 2, segW, segH);
      x += segW + SEG_GAP;
    }

    // Filled portion across segments
    const int fillUnits = (int)std::lround(clampf(s_disp_frac, 0.f, 1.f) * totalFillable);
    int remaining = fillUnits;
    x = BAR_X + 2;
    for (int i = 0; i < SEG_N && remaining > 0; ++i) {
      const int wDraw = (remaining >= segW) ? segW : remaining;
      const auto c = SEG_COLORS[i];
      EVE_SetFgColor(c.r, c.g, c.b);
      rect(x, BAR_Y + 2, wDraw, segH);
      remaining -= wDraw;
      x += segW + SEG_GAP;
    }

    // Soft-limit tick at (limit - buffer)
    const float softFrac  = clampf((ENERGY_LIMIT_KWH - ENERGY_BUFFER_KWH) / ENERGY_LIMIT_KWH, 0.f, 1.f);
    const int softUnits   = (int)std::lround(softFrac * totalFillable);
    const int softX       = fillUnitsToX(softUnits, segW);
    EVE_SetFgColor(80,80,80);
    rect(softX, BAR_Y - 8, 2, BAR_H + 16);

    // Over-limit blink banner below the bar
    if (s_disp_frac >= 1.0f && (now_ms % UI_BLINK_FAST_MS) < (UI_BLINK_FAST_MS/2)) {
      EVE_SetFgColor(210,40,30);
      rect(BAR_X, BAR_Y + BAR_H + 14, BAR_W, 28);
      EVE_SetFgColor(255,255,255);
      EVE_Text(BAR_X + BAR_W/2 - 70, BAR_Y + BAR_H + 34, FONT_LABEL, 0, "ENERGY LIMIT");
    }

    // Label
    EVE_SetFgColor(255,255,255);
    EVE_Text(BAR_X + BAR_W/2 - 50, BAR_Y - 14, FONT_LABEL, 0, "Energy Bar");
  }

} // namespace energybar

namespace lcd {

void render_energy_bar_demo(uint32_t now_ms) {
  energybar::update(now_ms);                 
  EVE_BeginFrame();
  EVE_SetBackColor(0, 0, 0);                  
  energybar::draw(now_ms); // draw bar in the middle of the screen          
  EVE_EndFrame();
}

void print_default_screen_template() {
  EVE_BeginFrame();
  EVE_SetBackColor(0, 0, 0);
  EVE_SetFgColor(255, 255, 255);
  EVE_Text(30, TFT_H - 60, FONT_MED, 0, "Created by: Aryan Singh");
  EVE_EndFrame();
}

void init() {
  EVE_Init();
  EVE_SetBackColor(0, 0, 0);
  EVE_SetFgColor(255, 255, 255); // MIGHT HAVE TO SET TO BLACK
}

void clear() {
  EVE_BeginFrame(); // hoping this driver func clears before beginning frame intake
  EVE_EndFrame();
}

void clear_section(uint8_t sect) {
  const int16_t* r = nullptr;
  switch (sect) {
    case 0: r = layout::SECT_HV;     break;
    case 1: r = layout::SECT_TPS0;   break;
    case 2: r = layout::SECT_TPS1;   break;
    case 3: r = layout::SECT_HVTEMP; break;
    case 4: r = layout::SECT_VNLOAD; break;
    default: return;
  }
  EVE_BeginFrame();
  EVE_RectFill(r[0], r[1], r[2], r[3]);
  EVE_EndFrame();
}

// Print functions to display on LCD

void print_hv(float hv) {
  if (changed_f(hv, hv_prev, 0.1f)) {
    hv_prev = hv;
    std::snprintf(s_hv, sizeof(s_hv), "%5.1f", hv);
  }
  render_base_screen();
}

void print_tps0percent(float p) {
  if (changed_f(p, tps0_prev, 0.05f)) {
    tps0_prev = p;
    std::snprintf(s_tps0, sizeof(s_tps0), "%0.1f", p);
  }
  render_base_screen();
}

void print_tps1percent(float p) {
  if (changed_f(p, tps1_prev, 0.05f)) {
    tps1_prev = p;
    std::snprintf(s_tps1, sizeof(s_tps1), "%3.1f", p);
  }
  render_base_screen();
}

void print_hvtemp(float t) {
  if (changed_f(t, hvtemp_prev, 0.1f)) {
    hvtemp_prev = t;
    leds::hvtemp(t);
    std::snprintf(s_hvtemp, sizeof(s_hvtemp), "%2.1f", t);
  }
  render_base_screen();
}

void print_hvlow(float v) {
  
  if (changed_f(v, hvlow_prev, 0.005f)) {
    hvlow_prev = v;
    std::snprintf(s_vnload, sizeof(s_vnload), "%1.2f", v);
  }
  render_base_screen();
}

// ADD FUNCS FOR OTHER STUFF THAT NEEDS TO BE SHOWN ON THE LCD SCREEN

// Diagnostics isnt called anywhere rn need to see if needed
void diagnostics(uint8_t cellfault, uint8_t cellwarn, uint8_t bmsstate) {
  std::snprintf(s_diag_cf,  sizeof(s_diag_cf),  "%hu", (unsigned short)cellfault);
  std::snprintf(s_diag_bms, sizeof(s_diag_bms), "%hu", (unsigned short)bmsstate);

  EVE_BeginFrame();
  draw_static_labels();
  draw_dynamic_values();
  EVE_Text(layout::DIAG_CF_X,  layout::DIAG_CF_Y,  FONT_LABEL, 0, s_diag_cf);
  EVE_Text(layout::DIAG_BMS_X, layout::DIAG_BMS_Y, FONT_LABEL, 0, s_diag_bms);
  EVE_EndFrame();
}

void print_rpm_diag(uint16_t rpm) {
  if (rpm != rpm_prev) {
    rpm_prev = rpm;
    std::snprintf(s_rpm, sizeof(s_rpm), "%5hu", rpm);
  }
  EVE_BeginFrame();
  draw_static_labels();
  // Show RPM in HV slot
  EVE_Text(layout::RPM_X, layout::RPM_Y, FONT_BIG, 0, s_rpm);
  EVE_Text(layout::TPS0_X,   layout::TPS0_Y,   FONT_MED, 0, s_tps0);
  EVE_Text(layout::TPS1_X,   layout::TPS1_Y,   FONT_MED, 0, s_tps1);
  EVE_Text(layout::HVTEMP_X, layout::HVTEMP_Y, FONT_MED, 0, s_hvtemp);
  EVE_Text(layout::VNLOAD_X, layout::VNLOAD_Y, FONT_MED, 0, s_vnload);
  EVE_EndFrame();
}

//Periodic update

void update_screenE(float hv, float tps0, float tps1, float hvtemp, uint32_t now_ms) {
  if (now_ms - prev_tick_ms < LCD_UPDATE_MS) return;
  prev_tick_ms = now_ms;

  // Option B
  // energybar::update(now_ms);

// REMOVE BPS AND ADD REGEN!!!!!!

  if (changed_f(hv, hv_prev, 0.1f)) {
    hv_prev = hv;
    std::snprintf(s_hv, sizeof(s_hv), "%5.1f", hv);
  }
  if (changed_f(tps0, tps0_prev, 0.05f)) {
    tps0_prev = tps0;
    std::snprintf(s_tps0, sizeof(s_tps0), "%0.1f", tps0);
  }
  if (changed_f(tps1, tps1_prev, 0.05f)) {
    tps1_prev = tps1;
    std::snprintf(s_tps1, sizeof(s_tps1), "%3.1f", tps1);
  }
  if (changed_f(hvtemp, hvtemp_prev, 0.1f)) {
    hvtemp_prev = hvtemp;
    leds::hvtemp(hvtemp);
    std::snprintf(s_hvtemp, sizeof(s_hvtemp), "%2.1f", hvtemp);
  }

  render_base_screen();
}

}
// =========== Stubs for EVE HAL Files, need to be updated with vendor provided HAL functions =============
/*
extern "C" void EVE_Init() {}
extern "C" void EVE_BeginFrame() {}
extern "C" void EVE_EndFrame() {}
extern "C" void EVE_SetBackColor(uint8_t, uint8_t, uint8_t) {}
extern "C" void EVE_SetFgColor(uint8_t, uint8_t, uint8_t) {}
extern "C" void EVE_Text(int16_t, int16_t, uint8_t, uint16_t, const char*) {}
extern "C" void EVE_RectFill(int16_t, int16_t, int16_t, int16_t) {}
*/
