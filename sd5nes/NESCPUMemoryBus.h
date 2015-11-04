#pragma once

#include "NESMemory.h"
#include "NESPPU.h"

#include "NESMemoryConstants.h"

/**
* Emulates the memory mapping of the NES CPU memory bus.
*/
class NESCPUMemoryBus : public INESMemoryInterface
{
public:
	NESCPUMemoryBus(NESPPU& ppu, NESMemory& ram, const NESMemory& prgRom);
	virtual ~NESCPUMemoryBus();

	/**
	* Writes 8-bits to the NES's CPU memory at a specified location with the specified value.
	* Memory will be mapped correctly.
	* Returns true on success, false on failure.
	*/
	bool Write8(u16 addr, u8 val) override;

	/**
	* Reads 8-bits from the NES's CPU memory at a specified location and modifies outVal (if not null).
	* Memory will be mapped correctly.
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read8(u16 addr, u8* outVal) const override;

	/**
	* Reads 16-bits from the NES's CPU memory at a specified location and modifies outVal (if not null).
	* Memory will be mapped correctly.
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read16(u16 addr, u16* outVal) const override;

private:
	NESPPU& ppu_;
	NESMemory& ram_;
	const NESMemory& prgRom_;

	// Handles the mirroring of a write if necessary.
	bool HandleWriteMirrors(u16 addr, u8 val);
};

