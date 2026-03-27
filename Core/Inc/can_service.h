// can_service.hpp
#pragma once
#include <cstdint>
#include "fdcan_bus.h"

namespace cansvc {


float hv();          // pack voltage
float hv_current();  // pack current
float hv_temp();     // pack temp (°C)
float hv_low();      // "No load" / LV as in original naming
float lv();          // low voltage battery
float soc();         // state of charge %
float tps0_percent();
float tps0_voltage();
float tps1_percent();
float tps1_voltage();
float bps0_percent();
float bps0_voltage();
float bps1_percent();
float bps1_voltage();

float rpm();
float bms_fault();
float bms_warn();
float bms_stat();
float can_service_get_energy_used_kWh();

// Service
bool  init(FdcanBus& bus);
void  poll(FdcanBus& bus);   
void  send_test(FdcanBus& bus);

} // namespace
