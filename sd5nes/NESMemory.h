#pragma once

#include <memory>

#include "NESTypes.h"
#include "NESMemoryConstants.h"

/* Different typedefs for the different memory types used by the NES. */
typedef NESMemory<NES_MEMORY_RAM_SIZE> NESMemoryRAM;
typedef NESMemory<NES_MEMORY_PRGROM_SIZE> NESMemoryPRGROM;
typedef NESMemory<NES_MEMORY_CHRROM_SIZE> NESMemoryCHRROM;

/**
* Represents the memory used by a hardware component of the NES system.
*/
template <uleast32 size>
class NESMemory
{
public:
	NESMemory() { }
	~NESMemory() { }

	/**
	* Sets all the allocated memory to zero.
	*/
	inline void ZeroMemory() { std::fill_n(data_, size, 0); }

	/**
	* Writes 8-bits to the memory at a specified location with the specified value.
	* Returns true on success, false on failure.
	*/
	bool Write8(u16 addr, u8 val)
	{
		if (addr >= size)
			return false;

		data_[addr] = val;
		return true;
	}

	/**
	* Reads 8-bits from the memory at a specified location and modifies outVal (if not null).
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read8(u16 addr, u8* outVal) const
	{
		if (addr >= size)
			return false;

		if (outVal != nullptr)
			*outVal = data_[addr];

		return true;
	}

	/**
	* Reads 16-bits from the memory at a specified location and modifies outVal (if not null).
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read16(u16 addr, u16* outVal) const
	{
		u8 low, hi;

		// Pull least-significant byte first.
		if (!Read8(addr, &low))
			return false;

		if (!Read8(addr + 1, &hi))
			return false;

		// Convert to 16-bit val.
		if (outVal != nullptr)
			*outVal = ((hi << 8) | low);

		return true;
	}

	// Get the allocated size of this memory.
	inline uleast32 GetSize() const { return size; }

private:
	u8 data_[size];
};

