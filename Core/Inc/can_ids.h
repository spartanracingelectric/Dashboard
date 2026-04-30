#pragma once
#include <cstdint>

// VCU-sourced
constexpr uint16_t CAN_TPS0          = 0x500; // byte[0] throttle%, scale 0.392157
constexpr uint16_t CAN_TPS1          = 0x501; // byte[0] throttle%, scale 0.392157
constexpr uint16_t CAN_BPS           = 0x502; // byte[0] brake%,    scale 0.392157
constexpr uint16_t CAN_LV_ADDR       = 0x507; // bytes[0:1] LV V (mV); bytes[2:3] regen torque s16; bytes[4:5] eff score s16×0.0001; byte[6] dash_mode
constexpr uint16_t CAN_DASH_FAULT    = 0x505;
constexpr uint16_t CAN_SOC           = CAN_LV_ADDR; // legacy alias — not used now
constexpr uint16_t CAN_EFF_SCORE_ADDR = CAN_LV_ADDR;
constexpr uint16_t CAN_PL            = 0x512; // byte[4] target power
constexpr uint16_t CAN_ENERGY_USED_ADDR = 0x6A1;
constexpr uint16_t CAN_RPM_ADDR      = 0x0A5;
constexpr uint16_t CAN_SHUNT_CURRENT      = 0x3F1; // bytes[0:3] s32 LE, mA
constexpr uint16_t CAN_SHUNT_VOLTAGE      = 0x3F3; // bytes[0:3] s32 LE, mV


// Custom_BMS
constexpr uint16_t CAN_BMS_SAFETY_CHECKER_ADDR = 0x600; // Safety_Checker: bytes[4:5] HVsens pack V × 0.01, bytes[6:7] Sum pack V × 0.01
constexpr uint16_t CAN_HV_ADDR                 = 0x622; // Pack_Summary_1: highest/lowest cell V (bytes 0-3), highest/lowest cell temp (bytes 4-5)
constexpr uint16_t CAN_BMS_SUMMARY_2_ADDR      = 0x623; // Pack_Summary_2: faults/warnings + pack SOC (bytes 6-7 u16 × 0.01)

// Legacy (unused)
constexpr uint16_t CAN_BMS_FAULT_ADDR= 0x001;
constexpr uint16_t CAN_BMS_WARN_ADDR = 0x002;
constexpr uint16_t CAN_BMS_STAT_ADDR = 0x003;
