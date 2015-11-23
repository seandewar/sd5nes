#include "NESMemory.h"

#include <sstream>


NESMemoryMapper::NESMemoryMapper()
{
}


NESMemoryMapper::~NESMemoryMapper()
{
}


void NESMemoryMapper::Write8(u16 addr, u8 val)
{
	const auto mapping = GetMapping(addr);
	mapping.first.Write8(mapping.second, val);
}


u8 NESMemoryMapper::Read8(u16 addr) const
{
	const auto mapping = GetMapping(addr);
	return mapping.first.Read8(mapping.second);
}