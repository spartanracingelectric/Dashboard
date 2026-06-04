#pragma once
#include <cstdint>

/* DIAGNOSTICS SCREEN*/

    // MCM-sourced
    constexpr uint16_t CAN_MCM_TEMP      = 0x0A0; // byte[0] module a
    constexpr uint16_t CAN_MOTOR_TEMP    = 0x0A2; // byte[4:5] motor temp

    // IMU-souced - TO DO

    // TireSensor-sourced
    constexpr uint16_t CAN_TIRE_FR      = 0x4B4; // from 4b4 to 4b7 is all fr
    constexpr uint16_t CAN_TIRE_RL      = 0x4B8; // from 4b8 to 4bb is all rl
    constexpr uint16_t CAN_TIRE_RR      = 0x4B8; // from 4bc to 4bf is all rr

    // BMS-sourced
    constexpr uint16_t CAN_BMS_SUMMARY_2_ADDR      = 0x623; // byte 16: pack imbalance mV

    // VCU-sourced
    constexpr uint16_t CAN_TERM_SENSE          = 0x509; // byte[0:1] hvil term



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
    constexpr uint16_t CAN_BMS_SUMMARY_2_ADDR      = 0x623; // byte 0: fault bits (see dash_fault.h)
