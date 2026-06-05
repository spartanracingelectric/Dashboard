#include "main.h"
#include "can_service.h"
#include "can_ids.h"
#include "config.h"
#include "timebase.h"
#include "leds.h"

static float s_curr_hv = 0, s_curr_hvlow = 0, s_curr_celltemp = 0;
static float s_curr_tps0p = 0, s_curr_tps1p = 0, s_curr_pl = 0;
static float s_curr_bms_fault = 0, s_curr_energy_pct = 0, s_dash_mode = 0;
static float s_shunt_voltage = 0, s_shunt_current = 0;
static float s_motor_temp = 0, s_mcu_temp = 0;
static float s_fr_temp = 0, s_rl_temp = 0, s_rr_temp = 0, s_long_g = 0;
static float s_lat_g = 0, s_pack_imbal = 0;
static float s_curr_slip = 0, s_slip_target = 0;
static bool  s_term_sense = false;

namespace cansvc {

float hv()           { return s_curr_hv; }
float hv_low()       { return s_curr_hvlow; }
float celltemp()     { return s_curr_celltemp; }
float tps0_percent() { return s_curr_tps0p; }
float tps1_percent() { return s_curr_tps1p; }
float pl()           { return s_curr_pl; }
float shunt_current(){ return s_shunt_current; }
float shunt_voltage(){ return s_shunt_voltage; }
float bms_fault()    { return s_curr_bms_fault; }
float energy_pct()   { return s_curr_energy_pct; }
float dash_mode()    { return s_dash_mode; }
float slip_ratio()   { return s_curr_slip; }
float slip_target()  { return s_slip_target; }
float motor_temp()   { return s_motor_temp;}
float mcu_temp()     { return s_mcu_temp;}
float tire_fr_temp() { return s_fr_temp;}
float tire_rl_temp() { return s_rl_temp;}
float tire_rr_temp() { return s_rr_temp;}
float long_g()       { return s_long_g;}
float lat_g()        { return s_lat_g;}
float pack_imbal()   { return s_pack_imbal;}
bool term_sense()    { return s_term_sense;}

bool init_vcu(FdcanBus& bus) {
  if (!bus.initClassic500k()) return false;
  const uint16_t ids[] = {
    CAN_TPS0, CAN_TPS1, CAN_LV_ADDR, CAN_PL, CAN_LC_STATUS_A,
    CAN_SHUNT_CURRENT, CAN_SHUNT_VOLTAGE,
    // diagnostics
    CAN_MCM_TEMP, CAN_MOTOR_TEMP, CAN_IMU_ACCEL,
    CAN_TIRE_FR, CAN_TIRE_RL, CAN_TIRE_RR, CAN_TERM_SENSE,
  };
  for (uint16_t id : ids) bus.addStdFilter(id);
  return true;
}

bool init_bms(FdcanBus& bus) {
  if (!bus.initClassic500k()) return false;
  const uint16_t ids[] = {
    CAN_BMS_SAFETY_CHECKER_ADDR, CAN_HV_ADDR, CAN_BMS_SUMMARY_2_ADDR,
  };
  for (uint16_t id : ids) bus.addStdFilter(id);
  return true;
}

static inline uint16_t u16(const uint8_t* d, int lo, int hi) {
  return (uint16_t)d[lo] | ((uint16_t)d[hi] << 8);
}
static inline uint32_t u32(const uint8_t* d, int b0, int b1, int b2, int b3) {
  return (uint32_t)d[b0] | ((uint32_t)d[b1] << 8) | ((uint32_t)d[b2] << 16) | ((uint32_t)d[b3] << 24);
}
static inline uint16_t be16(const uint8_t* d, int msb, int lsb) {
  return ((uint16_t)d[msb] << 8) | (uint16_t)d[lsb];
}
static inline float mag(float x) { return x < 0 ? -x : x; }
// Average the frame's 4 big-endian channels, then apply the DAQ scale (C).
static inline float tire_avg(const uint8_t* d) {
  uint32_t sum = (uint32_t)be16(d, 0, 1) + be16(d, 2, 3) + be16(d, 4, 5) + be16(d, 6, 7);
  return (sum * 0.25f) * 0.001f - 100.0f;
}

void poll(FdcanBus& bus) {
  CanFrame f{};
  while (bus.receive(f)) {
    const uint8_t* d = f.data;
    switch (f.id) {
      case CAN_BMS_SAFETY_CHECKER_ADDR:           // 0x600 Safety_Checker
        s_curr_hv = u16(d, 6, 7) * 0.01f;         // Sum_Pack_Voltage
        break;
      case CAN_HV_ADDR:                           // 0x622 Pack_Summary_1
        s_curr_hvlow    = u16(d, 2, 3) * 0.0001f; // Lowest_Cell_Voltage
        s_curr_celltemp = (int8_t)d[4];           // Highest_Cell_Temperature
        break;
      case CAN_BMS_SUMMARY_2_ADDR:                // 0x623 Pack_Summary_2
        s_curr_bms_fault = d[0];                  // fault bits (see dash_fault.h)
        s_pack_imbal = (int16_t)u16(d, 2, 3);
        break;
      case CAN_TPS0:
        s_curr_tps0p = d[0] * 0.392157f;
        break;
      case CAN_TPS1:
        s_curr_tps1p = d[0] * 0.392157f;
        break;
      case CAN_LV_ADDR:
        // 0x507 byte[4:5] eff score s16 × 0.0001 -> LED bar
        leds::efficiency_on_can_error((int16_t)u16(d, 4, 5) * 0.0001f);
        s_dash_mode = d[6];
        s_curr_energy_pct = d[7];
        break;
      case CAN_PL:
        s_curr_pl = d[4];
        break;
      case CAN_LC_STATUS_A:
        // 0x50B: [2:3] current slip, [6:7] target slip - both s16 LE ×1000
        s_curr_slip   = (int16_t)u16(d, 2, 3) * 0.001f;
        s_slip_target = (int16_t)u16(d, 6, 7) * 0.001f;
        break;
      case CAN_SHUNT_CURRENT:
        s_shunt_current = (int32_t)u32(d, 0, 1, 2, 3) * 0.001f; // mA -> A
        break;
      case CAN_SHUNT_VOLTAGE:
        s_shunt_voltage = (int32_t)u32(d, 0, 1, 2, 3) * 0.001f; // mV -> V
        break;
      case CAN_MCM_TEMP:
        s_mcu_temp = (int16_t)u16(d, 0, 1) * 0.1f;
        break;
      case CAN_MOTOR_TEMP:
        s_motor_temp = (int16_t)u16(d, 4, 5) * 0.1f;
        break;
      case CAN_IMU_ACCEL: {
        // hold the largest-magnitude G seen on each axis (sign preserved)
        float lg = (int16_t)u16(d, 0, 1) * 0.001f;
        float tg = (int16_t)u16(d, 2, 3) * 0.001f;
        if (mag(lg) > mag(s_long_g)) s_long_g = lg;
        if (mag(tg) > mag(s_lat_g))  s_lat_g  = tg;
        break;
      }
      case CAN_TIRE_FR:
        s_fr_temp = tire_avg(d);
        break;
      case CAN_TIRE_RL:
        s_rl_temp = tire_avg(d);
        break;
      case CAN_TIRE_RR:
        s_rr_temp = tire_avg(d);
        break;
      case CAN_TERM_SENSE:
        s_term_sense = (d[0] != 0);
        break;
      default:
        break;
    }
  }
}

void send_test(FdcanBus& bus) {
  static uint32_t beat = 0;
  if (!every_ms(beat, 200)) return;
  CanFrame f{};
  f.id = 0x7EE; f.len = 8; f.data[0] = 0x53;
  bus.send(f);
}

} // namespace
