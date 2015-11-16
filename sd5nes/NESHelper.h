#pragma once

#include <string>

#include "NESTypes.h"

/**
* Contains helper functions.
*/
namespace NESHelper
{
	// Checks whether or not addr1 and addr2 are in the same page of memory.
	inline bool IsInSamePage(u16 addr1, u16 addr2) { return ((addr1 & 0xFF00) == (addr2 & 0xFF00)); }

	// Converts two 8-bit values to one 16-bit value.
	inline u16 ConvertTo16(u8 hi, u8 lo) { return ((hi << 8) | lo); }
};