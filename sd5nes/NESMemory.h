#pragma once

#include "NESTypes.h"

/**
* Represents the memory used by the NES system.
*/
class NESMemory
{
public:
	NESMemory();
	~NESMemory();

	/*
	* Writes to the NES's memory at a specified location with the specified value.
	* Memory will be mapped correctly.
	* Returns true on success, false on failure.
	*/
	bool Write(u16 addr, u8 val);

	/*
	* Reads from the NES's memory at a specified location and modifies outVal (if not null).
	* Memory will be mapped correctly.
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read(u16 addr, u8* outVal);
};

