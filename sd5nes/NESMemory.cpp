#include "NESMemory.h"

#include <sstream>

#include "NESHelper.h"


NESMemoryMap::NESMemoryMap()
{
}


NESMemoryMap::~NESMemoryMap()
{
}


void NESMemoryMap::AddMemoryMapping(NESMemory& memory, u16 startAddr, u16 size)
{
	const auto info = NESMemoryMappingInfo(memory, startAddr, size);

	// Ensure no duplicate entries.
	assert("Duplicate mapping entry!" && 
		std::find(mappings_.begin(), mappings_.end(), info) == mappings_.end());

	mappings_.emplace_back(info);
}


void NESMemoryMap::AddMemoryMirror(u16 startAddr, u16 mirrorToAddr, u16 size)
{
	const auto info = NESMemoryMirroringInfo(startAddr, mirrorToAddr, size);

	// Ensure no duplicate entries.
	assert("Duplicate mirroring entry!" && 
		std::find(mirrors_.begin(), mirrors_.end(), info) == mirrors_.end());

	mirrors_.emplace_back(info);
}


void NESMemoryMap::AddMemoryMirrorRange(u16 startAddr, u16 endAddr, u16 mirrorToAddr, u16 mirrorEndAddr)
{
	const auto size = endAddr - startAddr + 1;
	const auto mirrorSize = mirrorEndAddr - mirrorToAddr + 1;

	// @TODO Support mirroring where the size of the mirror doesn't perfectly
	// divide the size of the data to be mirrored.
	const auto mirrorLoops = mirrorSize / size;
	for (int i = 0; i < mirrorLoops; ++i)
		AddMemoryMirror(startAddr, mirrorToAddr + (i * size), size);

	// If mirrorSize doesn't divide into size perfectly...
	if (mirrorSize % size != 0)
	{
		const auto lastMirrorStartAddr = mirrorToAddr + (mirrorLoops * size);
		AddMemoryMirror(startAddr, lastMirrorStartAddr, mirrorEndAddr - lastMirrorStartAddr + 1);
	}
}


std::pair<NESMemory&, u16> NESMemoryMap::GetMapping(u16 addr) const
{
	for (const auto& mapInfo : mappings_)
	{
		if (addr >= mapInfo.startAddr &&
			addr < mapInfo.startAddr + mapInfo.size)
			return std::make_pair(mapInfo.memory, addr - mapInfo.startAddr);
	}

	// No valid mapping for address!
	assert("Failed to lookup mapping - address not mapped!" && false);
	std::ostringstream oss;
	oss << "No mapping found for address 0x" << std::hex << addr;
	throw NESMemoryException(oss.str());
}


u16 NESMemoryMap::LookupMirrorAddress(u16 addr) const
{
	for (const auto& mirrorInfo : mirrors_)
	{
		if (addr >= mirrorInfo.mirrorToAddr &&
			addr < mirrorInfo.mirrorToAddr + mirrorInfo.size)
			return (mirrorInfo.startAddr + (addr - mirrorInfo.mirrorToAddr));
	}

	// Not a mirrored section of memory. Just give the original address.
	return addr;
}


void NESMemoryMap::Write8(u16 addr, u8 val)
{
	const auto mapping = GetMapping(LookupMirrorAddress(addr));
	mapping.first.Write8(mapping.second, val);
}


u8 NESMemoryMap::Read8(u16 addr) const
{
	const auto mapping = GetMapping(LookupMirrorAddress(addr));
	return mapping.first.Read8(mapping.second);
}


u16 NESMemoryMap::Read16(u16 addr) const
{
	const auto mapping = GetMapping(LookupMirrorAddress(addr));
	return mapping.first.Read16(mapping.second);
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

	for (uleast16 i = 0; i < data_.size(); ++i)
	{
		// Fill with data from buffer. If we have reached the end
		// of the buffer, fill the rest with zeros.
		data_[i] = (i < buf.size() ? buf[i] : 0);
	}
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