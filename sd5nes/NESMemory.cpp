#include "NESMemory.h"


NESMemory::NESMemory()
{
	data_ = std::unique_ptr<u8[]>(new u8[0xFFFF]);
}


NESMemory::~NESMemory()
{
}
