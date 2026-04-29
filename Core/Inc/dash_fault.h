#pragma once
#include <stdint.h>

const char* GetFaultName(uint32_t fault);

// Returns the name for a single fault bit, or nullptr if unknown.
const char* GetSingleFaultName(uint32_t fault_bit);
