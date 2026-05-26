#pragma once
#include <stdint.h>

// BMS fault bits mirrored from Pack_Summary_2 (0x623) byte 0.
constexpr uint8_t DF_CellUndervolt = 0x02; // bit 1
constexpr uint8_t DF_CellHighTemp  = 0x04; // bit 2
constexpr uint8_t DF_DisplayMask   = DF_CellUndervolt | DF_CellHighTemp;
