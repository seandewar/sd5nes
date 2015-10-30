#pragma once

#include "NESMemory.h"

#include "NESMemoryConstants.h"

/**
* Emulates the memory mapping of the NES memory bus.
*/
class NESMemoryBus
{
public:
	NESMemoryBus(NESMemory& ram, const NESMemory& prgRom);
	~NESMemoryBus();

	/**
	* Writes 8-bits to the NES's memory at a specified location with the specified value.
	* Memory will be mapped correctly.
	* Returns true on success, false on failure.
	*/
	bool Write8(u16 addr, u8 val);

	/**
	* Reads 8-bits from the NES's memory at a specified location and modifies outVal (if not null).
	* Memory will be mapped correctly.
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read8(u16 addr, u8* outVal) const;

	/**
	* Reads 16-bits from the NES's memory at a specified location and modifies outVal (if not null).
	* Memory will be mapped correctly.
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read16(u16 addr, u16* outVal) const;

private:
	NESMemory& ram_;
	const NESMemory& prgRom_;

	// Handles the mirroring of a write if necessary.
	bool HandleWriteMirrors(u16 addr, u8 val);
};

