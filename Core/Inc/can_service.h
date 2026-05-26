// can_service.hpp
#pragma once
#include <cstdint>
#include "fdcan_bus.h"

namespace cansvc {

float hv();            // pack voltage
float hv_low();        // lowest cell voltage (V)
float celltemp();      // highest cell temp (°C)
float tps0_percent();
float tps1_percent();
float pl();            // power limit
float shunt_current();
float shunt_voltage();
float bms_fault();     // Pack_Summary_2 byte 0 (see dash_fault.h)
float energy_pct();    // energy remaining %
float dash_mode();

bool  init_vcu(FdcanBus& bus);   // FDCAN2: TPS/LV/PL/Shunt
bool  init_bms(FdcanBus& bus);   // FDCAN1: Safety_Checker/Pack_Summary_1/2
void  poll(FdcanBus& bus);
void  send_test(FdcanBus& bus);

} // namespace
