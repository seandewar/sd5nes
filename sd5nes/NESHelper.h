#pragma once

#include <string>
#include <cassert>

#include "NESTypes.h"
#include "NESMemory.h"

/**
* Contains helper functions.
*/
namespace NESHelper
{
	/**
	* Gets the value of a bit in the specified position of an 8-bit value.
	*/
	inline bool IsBitSet(u8 val, u8 pos) 
	{
		assert(pos < 8); 
		return (((val >> pos) & 1) == 1); 
	}

	/**
	* Sets the bit at the specified position of an 8-bit value.
	*/
	inline void SetRefBit(u8& val, u8 pos) 
	{ 
		assert(pos < 8); 
		val |= (1 << pos); 
	}

	/**
	* Sets the bit at the specified position of an 8-bit value.
	* Returns the result and does not modify val.
	*/
	inline u8 SetBit(u8 val, u8 pos)
	{
		SetRefBit(val, pos);
		return val;
	}

	/**
	* Clear the bit at the specified position of an 8-bit value.
	*/
	inline void ClearRefBit(u8& val, u8 pos) 
	{ 
		assert(pos < 8); 
		val &= ~(1 << pos); 
	}

	/**
	* Clear the bit at the specified position of an 8-bit value.
	* Returns the result and does not modify val.
	*/
	inline u8 ClearBit(u8 val, u8 pos)
	{
		ClearRefBit(val, pos);
		return val;
	}

	/**
	* Sets/clears the bit at the specified position of an 8-bit value.
	*/
	inline void EditRefBit(u8& val, u8 pos, bool setBit) 
	{ 
		if (setBit)
			SetRefBit(val, pos);
		else
			ClearRefBit(val, pos); 
	}

	/**
	* Sets/clears the bit at the specified position of an 8-bit value.
	* Returns the result and does not modify val.
	*/
	inline u8 EditBit(u8 val, u8 pos, bool setBit)
	{
		EditRefBit(val, pos, setBit);
		return val;
	}

	/**
	* Checks whether or not addr1 and addr2 are in the same page of memory.
	*/
	inline bool IsInSamePage(u16 addr1, u16 addr2) { return ((addr1 & 0xFF00) == (addr2 & 0xFF00)); }

	/**
	* Converts two 8-bit values to one 16-bit value.
	*/
	inline u16 ConvertTo16(u8 hi, u8 lo) { return (hi << 8) | lo; }

	/**
	* Reads 16-bits (little-endian) from a memory interface at the specified address.
	*/
	inline u16 MemoryRead16(const INESMemoryInterface& mem, u16 addr) { return ConvertTo16(mem.Read8(addr + 1), mem.Read8(addr)); }

	/**
	* Reads 16-bits (little-endian) from a memory interface while accounting for the 6502 indirect addressing bug.
	* The bug causes the high byte of addr to not increment when the low byte wraps due to the increment in addr.
	*/
	inline u16 MemoryIndirectRead16(const INESMemoryInterface& mem, u16 addr) 
	{ 
		return ConvertTo16(mem.Read8((addr & 0xFF00) | ((addr + 1) & 0xFF)), mem.Read8(addr)); 
	}
};