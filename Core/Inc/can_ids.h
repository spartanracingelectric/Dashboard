#pragma once
#include <cstdint>

/* DIAGNOSTICS SCREEN*/

    // MCM-sourced
    constexpr uint16_t CAN_MCM_TEMP      = 0x0A0; // [0:1] module A temp, int16 ×0.1 C
    constexpr uint16_t CAN_MOTOR_TEMP    = 0x0A2; // [4:5] motor temp, int16 ×0.1 C

    // IMU-sourced (custom DAQ->dash). LE int16 ×0.001 G: [0:1] long G, [2:3] lat G
    constexpr uint16_t CAN_IMU_ACCEL     = 0x4C0;

    // TireSensor-sourced (DAQ). Big-endian Ch1-4 frame, avg ×0.001 - 100 C. FL (0x4B0) dead, unused.
    constexpr uint16_t CAN_TIRE_FR      = 0x4B4;
    constexpr uint16_t CAN_TIRE_RL      = 0x4B8;
    constexpr uint16_t CAN_TIRE_RR      = 0x4BC;

    // VCU-sourced
    constexpr uint16_t CAN_TERM_SENSE   = 0x509; // [0] HVIL (1 = HV present)



/* NORMAL SCREEN*/

    // VCU-sourced
    constexpr uint16_t CAN_TPS0          = 0x500; // byte[0] throttle%, scale 0.392157
    constexpr uint16_t CAN_TPS1          = 0x501; // byte[0] throttle%, scale 0.392157
    constexpr uint16_t CAN_LV_ADDR       = 0x507; // [4:5] eff score s16×0.0001; [7] energy %
    constexpr uint16_t CAN_BPS1          = 0x50D; // byte[1] dash_mode (LCD brightness, 1..6)
    constexpr uint16_t CAN_PL            = 0x512; // byte[4] target power
    constexpr uint16_t CAN_SHUNT_CURRENT = 0x3F1; // [0:3] s32 LE, mA
    constexpr uint16_t CAN_SHUNT_VOLTAGE = 0x3F3; // [0:3] s32 LE, mV

    // BMS-sourced
    constexpr uint16_t CAN_BMS_SAFETY_CHECKER_ADDR = 0x600; // [6:7] Sum pack V × 0.01
    constexpr uint16_t CAN_HV_ADDR                 = 0x622; // [2:3] lowest cell V × 0.0001; [4] highest cell temp s8
    constexpr uint16_t CAN_BMS_SUMMARY_2_ADDR      = 0x623; // [0] fault bits (dash_fault.h); [2:3] pack imbalance mV int16
