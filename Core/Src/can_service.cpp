#include "main.h"
#include "can_service.h"
#include "can_ids.h"
#include "config.h"
#include "timebase.h"
#include "leds.h"
#include <cstring>

//Internal state
static float s_curr_hv=0, s_curr_soc=0, s_curr_lv=0, s_curr_hvlow=0, s_curr_celltemp=0, s_curr_celltemp_low=0;
static float s_curr_hv_current=0, s_curr_pl=0, s_curr_bps=0, s_curr_tps0p=0, s_curr_tps1p=0;
static float s_curr_rpm=0, s_curr_bms_fault=0,  s_curr_bms_warn=0, s_curr_bms_stat=0;
static float s_curr_energy_pct=0;
static float s_curr_pl_tq=0;
static float s_energy_used_kWh = 0.0f;
static uint32_t s_energy_ts_ms    = 0;

/* Parameters for Dash Fault */
static float s_fault_code = 0, s_source = 0, s_context = 0;

// static float s_energy_used_kWh = 3.0f; // Half full sample start for energy bar

//Getters
#define DEF_GET(name) float name(){ return s_##name; }
namespace cansvc {
  DEF_GET(curr_hv)     
}
#undef DEF_GET
// Getter functions, does the same task as getter functions in SR-16 code
namespace cansvc {
float hv()          { return s_curr_hv; }
float hv_current()  { return s_curr_hv_current; }
float hv_temp()     { return s_curr_celltemp; }
float hv_low()      { return s_curr_hvlow; }
float lv()          { return s_curr_lv; }
float soc()         { return s_curr_soc; }
float tps0_percent(){ return s_curr_tps0p; }
float tps0_voltage(){ return 0.0f; }
float tps1_percent(){ return s_curr_tps1p; }
float tps1_voltage(){ return 0.0f; }
float pl()           { return s_curr_pl; }
float celltemp()     { return s_curr_celltemp; }
float celltemp_low() { return s_curr_celltemp_low; }
float dash_fault_code() { return s_fault_code; }
float dash_fault_source() { return s_source; }
float dash_fault_context() { return s_context; }
float pl_tq() 		 {return s_curr_pl_tq;}

float bps_percent(){ return s_curr_bps; }
float rpm()         { return s_curr_rpm; }
float bms_fault()   { return s_curr_bms_fault; }
float bms_warn()    { return s_curr_bms_warn; }
float bms_stat()    { return s_curr_bms_stat; }
float energy_pct()  { return s_curr_energy_pct; }
float can_service_get_energy_used_kWh() { return s_energy_used_kWh; }

//Filters & init
bool init_vcu(FdcanBus& bus) {
  if (!bus.initClassic500k()) return false;
  const uint16_t ids[] = {
    CAN_TPS0, CAN_TPS1, CAN_BPS, CAN_LV_ADDR, CAN_PL, CAN_ENERGY_USED_ADDR,
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

//Polling & parsing
static inline uint16_t u16(const uint8_t* d, int lo, int hi) {
  return (uint16_t)d[lo] | ((uint16_t)d[hi] << 8);
}
static inline uint32_t u32(const uint8_t* d, int b0, int b1, int b2, int b3) {
  return (uint32_t)d[b0] | ((uint32_t)d[b1] << 8) | ((uint32_t)d[b2] << 16) | ((uint32_t)d[b3] << 24);
}
// NEED TO WORK ON ENDIANNESS ISSUE, IF I HAVE ENDIANNESS OF EACH INPUT CORRECT

void poll(FdcanBus& bus) {
  CanFrame f{};
  while (bus.receive(f)) {
    const uint8_t* d = f.data;

    switch (f.id) {
      case CAN_BMS_SAFETY_CHECKER_ADDR:                 // Custom_BMS Safety_Checker (0x600)
        s_curr_hv = u16(d, 6, 7) * 0.01f;               // Sum_Pack_Voltage: bytes 6-7, u16 × 0.01 V
        break;
      case CAN_HV_ADDR:                                 // Custom_BMS Pack_Summary_1 (0x622)
        s_curr_hvlow        = u16(d,2,3) * 0.0001f;          // Lowest_Cell_Voltage: bytes 2-3, u16 × 0.0001 V
        s_curr_celltemp     = (int8_t)d[4];                  // Highest_Cell_Temperature: byte 4, s8 × 1 °C
        s_curr_celltemp_low = (int8_t)d[5];                  // Lowest_Cell_Temperature:  byte 5, s8 × 1 °C
        break;
      case CAN_BMS_SUMMARY_2_ADDR: // Custom_BMS Pack_Summary_2 (0x623)
        s_curr_bms_fault = d[0];                        // bits 0-7: 8× fault flags
        s_curr_bms_warn  = d[1];                        // bits 8-15: warning flags
        s_curr_soc       = u16(d, 6, 7) * 0.01f;        // State_of_Charge: bytes 6-7, u16 × 0.01 (for future use)
        break;
      case CAN_TPS0:
        s_curr_tps0p = d[0] * 0.392157f;
        break;
      case CAN_TPS1:
        s_curr_tps1p = d[0] * 0.392157f;
        break;
      case CAN_BPS:
        s_curr_bps = d[0] * 0.392157f;
        break;
      case CAN_LV_ADDR:
        s_curr_lv  = u16(d, 0, 1) * 0.001f;  // bytes 0-1: LV voltage mV → V
        s_curr_energy_pct = d[7];            // byte 7: energy % (0-100)
        // bytes 5-6: Eff Score, s16 × 0.0001
        leds::efficiency_on_can_error((int16_t)u16(d, 5, 6) * 0.0001f);
        break;
      case CAN_PL:
        s_curr_pl = d[4];
        s_curr_pl_tq = (int16_t)u16(d, 2, 3);  // sbyte2 Nm
        break;
      case CAN_ENERGY_USED_ADDR: {
          s_curr_energy_pct = d[0]; // 6 sent = 6% directly
          s_energy_ts_ms = now_ms();
          break;
      }
      case CAN_DASH_FAULT:
        s_fault_code = d[0];
        s_source = d[1];
        s_context = d[2];
        break;
      default:
        break;
    }
  }
}

// Optional test tx
void send_test(FdcanBus& bus) {
  static uint32_t beat = 0;
  if (!every_ms(beat, 200)) return;
  CanFrame f{};
  f.id = 0x7EE; f.len = 8; f.data[0] = 0x53;
  bus.send(f);
}

} // namespace

extern "C" float can_service_get_energy_used_kWh() {
  return s_energy_used_kWh;
}
