#pragma once
#include <cstdint>

constexpr uint16_t CAN_TPS0          = 0x500; // done
constexpr uint16_t CAN_TPS1          = 0x501; // done
constexpr uint16_t CAN_BPS           = 0x502; // done
constexpr uint16_t CAN_BAT_TEMP_ADDR = 0x50E; // done — VCU 0x50E bytes[3:4] = highestCellTemp
constexpr uint16_t CAN_PL            = 0x512; // done
constexpr uint16_t CAN_SOC           = 0x507; // done — VCU 0x507 byte[2] = LV SOC
constexpr uint16_t CAN_HV_ADDR       = 0x629; // BMS details — pack voltage
constexpr uint16_t CAN_BMS_FAULT_ADDR= 0x001;
constexpr uint16_t CAN_BMS_WARN_ADDR = 0x002;
constexpr uint16_t CAN_BMS_STAT_ADDR = 0x003;
constexpr uint16_t CAN_HV_CURRENT_ADDR = 0x004;
constexpr uint16_t CAN_ENERGY_USED_ADDR =0x6A1;
constexpr uint16_t CAN_EFF_SCORE_ADDR = CAN_SOC; // both are on the same id

constexpr uint16_t CAN_RPM_ADDR      = 0x0A5;
