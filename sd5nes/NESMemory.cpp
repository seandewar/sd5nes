#include "NESMemory.h"


NESMemoryMapper::NESMemoryMapper()
{
}


NESMemoryMapper::~NESMemoryMapper()
{
}


void NESMemoryMapper::Write8(u16 addr, u8 val)
{
	const auto mapping = GetMapping(addr);
	if (mapping.first != nullptr)
		mapping.first->Write8(mapping.second, val);
	else
	{
		// Invalid mapping if we got here.
		assert(false && "Attempt to write to address that points to no mapping!");
	}
}


u8 NESMemoryMapper::Read8(u16 addr) const
{
	const auto mapping = GetMapping(addr);
	if (mapping.first != nullptr)
		return mapping.first->Read8(mapping.second);

	assert(false && "Attempt to read data from address that points to no mapping!");
	return 0;
}