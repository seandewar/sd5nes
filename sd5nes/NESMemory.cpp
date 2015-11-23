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
	if (mapping.first == nullptr)
		throw NESMemoryException("Attempted to write to unmapped memory!");
		
	mapping.first->Write8(mapping.second, val);
}


u8 NESMemoryMapper::Read8(u16 addr) const
{
	const auto mapping = GetMapping(addr);
	if (mapping.first == nullptr)
		throw NESMemoryException("Attemptted to read unmapped memory!");
		
	return mapping.first->Read8(mapping.second);
}