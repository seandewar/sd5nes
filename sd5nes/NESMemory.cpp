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
}


u8 NESMemoryMapper::Read8(u16 addr) const
{
	const auto mapping = GetMapping(addr);
	if (mapping.first != nullptr)
		return mapping.first->Read8(mapping.second);
}