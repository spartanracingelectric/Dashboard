#pragma once
#include <cstdint>

// VCU-sourced
constexpr uint16_t CAN_TPS0          = 0x500; // byte[0] throttle%, scale 0.392157
constexpr uint16_t CAN_TPS1          = 0x501; // byte[0] throttle%, scale 0.392157
constexpr uint16_t CAN_LV_ADDR       = 0x507; // [4:5] eff score s16×0.0001; [7] energy %
constexpr uint16_t CAN_BPS1          = 0x50D; // byte[1] dash_mode (LCD brightness, 1..6)
constexpr uint16_t CAN_PL            = 0x512; // byte[4] target power
constexpr uint16_t CAN_SHUNT_CURRENT = 0x3F1; // [0:3] s32 LE, mA
constexpr uint16_t CAN_SHUNT_VOLTAGE = 0x3F3; // [0:3] s32 LE, mV

// Custom_BMS
constexpr uint16_t CAN_BMS_SAFETY_CHECKER_ADDR = 0x600; // [6:7] Sum pack V × 0.01
constexpr uint16_t CAN_HV_ADDR                 = 0x622; // [2:3] lowest cell V × 0.0001; [4] highest cell temp s8
constexpr uint16_t CAN_BMS_SUMMARY_2_ADDR      = 0x623; // byte 0: fault bits (see dash_fault.h)
