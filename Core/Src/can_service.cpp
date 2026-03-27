#include "main.h"
#include "can_service.h"
#include "can_ids.h"
#include "config.h"
#include "timebase.h"
#include "leds.h"
#include <cstring>

//Internal state
static float s_curr_hv=0, s_curr_soc=0, s_curr_lv=0, s_curr_hvlow=0, s_curr_hvtemp=0;
static float s_curr_hv_current=0, s_curr_tps0v=0, s_curr_tps0p=0, s_curr_tps1v=0, s_curr_tps1p=0;
static float s_curr_rpm=0, s_curr_bms_fault=0, s_curr_bms_warn=0, s_curr_bms_stat=0;
static float    s_energy_used_kWh = 0.0f;   
static uint32_t s_energy_ts_ms    = 0;
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
float hv_temp()     { return s_curr_hvtemp; }
float hv_low()      { return s_curr_hvlow; }
float lv()          { return s_curr_lv; }
float soc()         { return s_curr_soc; }
float tps0_percent(){ return s_curr_tps0p; }
float tps0_voltage(){ return s_curr_tps0v; }
float tps1_percent(){ return s_curr_tps1p; }
float tps1_voltage(){ return s_curr_tps1v; }

float bps0_percent(){ return s_curr_bps0p; }
float bps0_voltage(){ return s_curr_bps0v; }
float bps1_percent(){ return s_curr_bps1p; }
float bps1_voltage(){ return s_curr_bps1v; }
float rpm()         { return s_curr_rpm; }
float bms_fault()   { return s_curr_bms_fault; }
float bms_warn()    { return s_curr_bms_warn; }
float bms_stat()    { return s_curr_bms_stat; }

//Filters & init
bool init(FdcanBus& bus) {
  if (!bus.initClassic500k()) return false;

  const uint16_t ids[] = {
    CAN_TPS0, CAN_TPS1, CAN_HV_ADDR, CAN_BAT_TEMP_ADDR,
    CAN_LV_ADDR, CAN_RPM_ADDR, CAN_SOC_ADDR,CAN_BMS_FAULT_ADDR,
    CAN_BMS_WARN_ADDR,CAN_BMS_STAT_ADDR
  };
  for (uint16_t id : ids) bus.addStdFilter(id);
  bus.addStdFilter(CAN_ENERGY_USED_ADDR);
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
      case CAN_LV_ADDR:           // 0x507: LV (bytes 0-1), eff score (bytes 6-7)
        s_curr_lv = u16(d,0,1) * 0.001f;
        { int16_t cp = (int16_t)u16(d, 6, 7);
          leds::efficiency_on_can_error(float(cp) / 10000.0f); }
        break;
      case CAN_HV_ADDR:           // curr_hv = (b4..b7) * 0.001f
        s_curr_hv = u32(d,4,5,6,7) * 0.001f;                                          
        s_curr_hvlow = u16(d,4,5) * 0.001f;     
        s_curr_hv_current = u32(d,0,1,2,3) * 0.001f; // if same frame is used       
        break;
      case CAN_TPS0:
        s_curr_tps0p = d[0] * 0.392156862746f; // 100/255  kB to Percent conversion                            
        s_curr_tps0v = u16(d,2,3) * 0.001f;                                       
        break;
      case CAN_TPS1:
        s_curr_tps1p = d[0] * 0.392156862746f;                                       
        s_curr_tps1v = u16(d,2,3) * 0.001f;                                        
        break;
      case CAN_BAT_TEMP_ADDR:
        s_curr_hvtemp = ( (uint16_t)d[7] << 8 | d[6] ) * 0.1f;                       
        break;
      case CAN_RPM_ADDR:
        s_curr_rpm = u16(d,2,1);                                                     
        break;
      case CAN_SOC_ADDR:
        //curr_soc = ((d[6]) | (d[7] << 8)) * 0.1f;
        s_curr_soc = ((uint16_t)d[6] | ((uint16_t)d[7] << 8)) * 0.1f;
        break;
      case CAN_BMS_FAULT_ADDR:
        s_curr_bms_fault = d[1];
        break;
      case CAN_BMS_WARN_ADDR:
        s_curr_bms_warn = d[1];
        break;
      case CAN_BMS_STAT_ADDR:
        s_curr_bms_stat = d[6];
        break;
    //case CAN_HV_CURRENT_ADDR:
    //  s_curr_hv_current = u32(d,0,1,2,3) * 0.001f;
    //  break;
      case CAN_ENERGY_USED_ADDR: {
        
        const uint32_t wh  = u32(d, 0, 1, 2, 3);
        const float    kWh = wh * 0.001f;   // Wh to kWh

        if (kWh >= s_energy_used_kWh - 0.1f) {
          s_energy_used_kWh = kWh;
        }
        s_energy_ts_ms = now_ms();
        break;
      }

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
