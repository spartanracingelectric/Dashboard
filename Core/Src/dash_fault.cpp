#include "dash_fault.h"

// Faults - took from safety.c in the VCU
static const uint32_t F_tpsOutOfRange = 0x1;
static const uint32_t F_bpsOutOfRange = 0x2;
static const uint32_t F_tpsPowerFailure = 0x4;
static const uint32_t F_bpsPowerFailure = 0x8;
static const uint32_t F_tpsSignalFailure = 0x10;
static const uint32_t F_bpsSignalFailure = 0x20;
static const uint32_t F_tpsNotCalibrated = 0x40;
static const uint32_t F_bpsNotCalibrated = 0x80;
static const uint32_t F_tpsOutOfSync = 0x100;
static const uint32_t F_bpsOutOfSync = 0x200;
static const uint32_t F_tpsbpsImplausible = 0x400;

const char* GetFaultName(uint32_t fault)
{
    if (fault == 0)                        return "No Fault";
    if (fault & F_tpsbpsImplausible)       return "TPS/BPS Implausible";
    if (fault & F_tpsPowerFailure)         return "TPS Power Failure";
    if (fault & F_bpsPowerFailure)         return "BPS Power Failure";
    if (fault & F_tpsSignalFailure)        return "TPS Signal Failure";
    if (fault & F_bpsSignalFailure)        return "BPS Signal Failure";
    if (fault & F_tpsOutOfRange)           return "TPS Out of Range";
    if (fault & F_bpsOutOfRange)           return "BPS Out of Range";
    if (fault & F_tpsOutOfSync)            return "TPS Out of Sync";
    if (fault & F_bpsOutOfSync)            return "BPS Out of Sync";
    if (fault & F_tpsNotCalibrated)        return "TPS Not Calibrated";
    if (fault & F_bpsNotCalibrated)        return "BPS Not Calibrated";
    return "Unknown Fault";
}

const char* GetSingleFaultName(uint32_t fault_bit)
{
    if (fault_bit == F_tpsbpsImplausible)   return "TPS/BPS Implausible";
    if (fault_bit == F_tpsPowerFailure)     return "TPS Power Failure";
    if (fault_bit == F_bpsPowerFailure)     return "BPS Power Failure";
    if (fault_bit == F_tpsSignalFailure)    return "TPS Signal Failure";
    if (fault_bit == F_bpsSignalFailure)    return "BPS Signal Failure";
    if (fault_bit == F_tpsOutOfRange)       return "TPS Out of Range";
    if (fault_bit == F_bpsOutOfRange)       return "BPS Out of Range";
    if (fault_bit == F_tpsOutOfSync)        return "TPS Out of Sync";
    if (fault_bit == F_bpsOutOfSync)        return "BPS Out of Sync";
    if (fault_bit == F_tpsNotCalibrated)    return "TPS Not Calibrated";
    if (fault_bit == F_bpsNotCalibrated)    return "BPS Not Calibrated";
    return nullptr;
}