#include "NESMemory.h"

#include <cassert>
#include <sstream>

#include "NESHelper.h"


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
	assert("Duplicate entry!" && 
		std::find(mappings_.begin(), mappings_.end(), info) == mappings_.end());

	mappings_.emplace_back(info);
}


void NESMemoryMap::AddMemoryMirror(u16 startAddr, u16 mirrorToAddr, u16 size)
{
	const auto info = NESMemoryMirroringInfo(startAddr, mirrorToAddr, size);

	// Ensure no duplicate entries.
	assert("Duplicate entry!" && 
		std::find(mirrors_.begin(), mirrors_.end(), info) == mirrors_.end());

	mirrors_.emplace_back(info);
}


std::pair<NESMemory&, u16> NESMemoryMap::LookupMapping(u16 addr)
{
	for (const auto& mapInfo : mappings_)
	{
		if (addr >= mapInfo.startAddr &&
			addr <= mapInfo.startAddr + mapInfo.memory.GetSize())
			return std::make_pair(mapInfo.memory, addr - mapInfo.startAddr);
	}

	// No valid mapping for address!
	assert("Failed to lookup mapping - address not mapped!" && false);
	std::ostringstream oss;
	oss << "No mapping found for address 0x" << std::hex << addr;
	throw NESMemoryException(oss.str());
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
	return NESHelper::ConvertTo16(Read8(addr + 1), Read8(addr));
}