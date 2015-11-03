#pragma once

#include <vector>

#include "NESTypes.h"
#include "NESMemoryConstants.h"

/**
* Represents an interface for writing and reading from a memory buffer.
*/
class INESMemoryInterface
{
public:
	virtual bool Write8(u16 addr, u8 val) = 0;

	virtual bool Read8(u16 addr, u8* outVal) const = 0;
	virtual bool Read16(u16 addr, u16* outVal) const = 0;
};

/**
* Represents the memory used by a hardware component of the NES system.
*/
class NESMemory : public INESMemoryInterface
{
public:
	NESMemory(uleast16 size = 0);
	NESMemory(const std::vector<u8>& data);
	virtual ~NESMemory();

	/**
	* Sets all the allocated memory to zero.
	*/
	void ZeroMemory();

	/**
	* Writes 8-bits to the memory at a specified location with the specified value.
	* Returns true on success, false on failure.
	*/
	bool Write8(u16 addr, u8 val) override;

	/**
	* Reads 8-bits from the memory at a specified location and modifies outVal (if not null).
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read8(u16 addr, u8* outVal) const override;

	/**
	* Reads 16-bits from the memory at a specified location and modifies outVal (if not null).
	* Returns true on success, false on failure. If failure, outVal is not modified.
	*/
	bool Read16(u16 addr, u16* outVal) const override;

	// Get the allocated size of this memory.
	uleast32 GetSize() const;

private:
	std::vector<u8> data_;
};

