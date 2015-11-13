#include "NESMemory.h"

#include <cassert>


NESMemoryMap::NESMemoryMap()
{
}


NESMemoryMap::~NESMemoryMap()
{
}


void NESMemoryMap::AddMemoryMapping(NESMemory& memory, u16 startAddr)
{
	const auto info = NESMemoryMappingInfo(startAddr, memory);

	// Ensure no duplicate entries.
	assert(std::find(mappings_.begin(), mappings_.end(), info) == mappings_.end());

	mappings_.emplace_back(info);
}


void NESMemoryMap::AddMemoryMirror(u16 startAddr, u16 mirrorToAddr, u16 size)
{
	const auto info = NESMemoryMirroringInfo(startAddr, mirrorToAddr, size);

	// Ensure no duplicate entries.
	assert(std::find(mirrors_.begin(), mirrors_.end(), info) == mirrors_.end());

	mirrors_.emplace_back(info);
}


void NESMemoryMap::Write8(u16 addr, u8 val)
{
	//@TODO
}


u8 NESMemoryMap::Read8(u16 addr) const
{
	//@TODO
}


u16 NESMemoryMap::Read16(u16 addr) const
{
	//@TODO
}


NESMemory::NESMemory(uleast16 size) :
data_(size)
{
	ZeroMemory();
}


NESMemory::~NESMemory()
{
}


void NESMemory::CopyFromBuffer(const std::vector<u8>& buf)
{
	// Copying from a buffer which is larger than ours
	// is probably bad...
	assert(buf.size() <= data_.size());

	ZeroMemory();
	for (uleast16 i = 0; i < buf.size(); ++i)
		data_[i] = buf[i];
}


void NESMemory::Write8(u16 addr, u8 val)
{
	if (addr >= data_.size())
		throw NESMemoryException("Cannot write to memory outside of allocated space!");

	data_[addr] = val;
}


u8 NESMemory::Read8(u16 addr) const
{
	if (addr >= data_.size())
		throw NESMemoryException("Cannot read from memory outside of allocated space!");

	return data_[addr];
}


u16 NESMemory::Read16(u16 addr) const
{
	// Convert to 16-bit val.
	return ((Read8(addr + 1) << 8) | Read8(addr));
}