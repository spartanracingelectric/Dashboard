#pragma once
#include <cstdint>

constexpr uint16_t CAN_TPS0          = 0x500;
constexpr uint16_t CAN_TPS1          = 0x501;
constexpr uint16_t CAN_LV_ADDR       = 0x507;
constexpr uint16_t CAN_HV_ADDR       = 0x620;
constexpr uint16_t CAN_BAT_TEMP_ADDR = 0x623;
constexpr uint16_t CAN_SOC_ADDR      = 0x000;
constexpr uint16_t CAN_BMS_FAULT_ADDR= 0x001;
constexpr uint16_t CAN_BMS_WARN_ADDR = 0x002;
constexpr uint16_t CAN_BMS_STAT_ADDR = 0x003;
constexpr uint16_t CAN_HV_CURRENT_ADDR = 0x004;
constexpr uint16_t CAN_ENERGY_USED_ADDR =0x6A1;
constexpr uint16_t CAN_EFF_SCORE_ADDR = CAN_LV_ADDR; // both are on the same id

constexpr uint16_t CAN_RPM_ADDR      = 0x0A5;
