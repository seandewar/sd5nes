#include "NESMemory.h"


NESMemory::NESMemory()
{
	data_ = std::unique_ptr<u8[]>(new u8[0xFFFF]);
}


NESMemory::~NESMemory()
{
}


bool NESMemory::Write8(u16 addr, u8 val)
{
	return false;
}


bool NESMemory::Read8(u16 addr, u8* outVal) const
{
	return false;
}


bool NESMemory::Read16(u16 addr, u16* outVal) const
{
	return false;
}
