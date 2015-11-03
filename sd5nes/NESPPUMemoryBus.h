#pragma once

#include "NESMemory.h"

#include <array>

#include "NESMemoryConstants.h"

/**
* Emulates the memory mapping of the NES PPU memory bus.
*/
class NESPPUMemoryBus : public INESMemoryInterface
{
public:
	NESPPUMemoryBus(
		std::array<NESMemory, 2>& patternTables, 
		std::array<NESMemory, 4>& nameTables, 
		std::array<NESMemory, 4>& attribTables,
		NESMemory& paletteMem
		);
	virtual ~NESPPUMemoryBus();

	/**
	* Writes 8-bits to the NES's PPU memory at a specified location with the specified value.
	* Memory will be mapped correctly.
	* Returns true on success, false on failure.
	*/
	bool Write8(u16 addr, u8 val) override;

	/**
	* Reads 8-bits from the NES's PPU memory at a specified location and modifies outVal (if not null).
	* Memory will be mapped correctly.
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read8(u16 addr, u8* outVal) const override;

	/**
	* Reads 16-bits from the NES's PPU memory at a specified location and modifies outVal (if not null).
	* Memory will be mapped correctly.
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read16(u16 addr, u16* outVal) const override;

private:
	std::array<NESMemory, 2>& patternTables_;
	std::array<NESMemory, 4>& nameTables_;
	std::array<NESMemory, 4>& attribTables_;
	NESMemory& paletteMem_;

	// Handles the mirroring of a write if necessary.
	bool HandleWriteMirrors(u16 addr, u8 val);
};

