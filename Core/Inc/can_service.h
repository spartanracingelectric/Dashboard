// can_service.hpp
#pragma once
#include <cstdint>
#include "fdcan_bus.h"

namespace cansvc {


float hv();          // pack voltage
float hv_current();  // pack current
float hv_temp();     // pack temp (°C)
float hv_low();      // lowest cell voltage (V)
float lv();          // low voltage battery
float soc();         // state of charge %
float tps0_percent();
float tps0_voltage();
float tps1_percent();
float tps1_voltage();
float bps_percent();
float pl_tq();
float pl();           // power limit
float rpm();
float celltemp();      // highest cell temp (°C)
float celltemp_low();  // lowest cell temp (°C)
float bms_fault();
float bms_warn();
float bms_stat();
float energy_pct();  // energy percentage from VCU
float can_service_get_energy_used_kWh();

// Service
bool  init_vcu(FdcanBus& bus);   // FDCAN2: TPS/BPS/LV/PL/Energy
bool  init_bms(FdcanBus& bus);   // FDCAN1: Safety_Checker/Pack_Summary_1/2
void  poll(FdcanBus& bus);       // call on each bus
void  send_test(FdcanBus& bus);

} // namespace
