#include "NESCPUMemoryBus.h"


NESCPUMemoryBus::NESCPUMemoryBus(NESMemory& ram, const NESMemory& prgRom) :
ram_(ram),
prgRom_(prgRom)
{
}


NESCPUMemoryBus::~NESCPUMemoryBus()
{
}


bool NESCPUMemoryBus::Write8(u16 addr, u8 val)
{
	// Handle mirroring if necessary.
	if (!HandleWriteMirrors(addr, val))
		return false;

	if (addr >= NES_MEMORY_RAM_START &&
		addr <= NES_MEMORY_RAM_END)
	{
		// Write to CPU RAM.
		if (!ram_.Write8(addr - NES_MEMORY_RAM_START, val))
			return false;
	}
	else
	{
		// @TODO Implement bus fully.
		return false;
	}

	return true;
}


bool NESCPUMemoryBus::Read8(u16 addr, u8* outVal) const
{
	if (addr >= NES_MEMORY_RAM_START &&
		addr <= NES_MEMORY_RAM_END)
	{
		// Read from CPU RAM.
		if (!ram_.Read8(addr - NES_MEMORY_RAM_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PRGROM_START &&
		addr <= NES_MEMORY_PRGROM_END)
	{
		// Read from PRG-ROM.
		if (!prgRom_.Read8(addr - NES_MEMORY_PRGROM_START, outVal))
			return false;
	}
	else
	{
		// @TODO Implement bus fully.
		return false;
	}

	return true;
}


bool NESCPUMemoryBus::Read16(u16 addr, u16* outVal) const
{
	if (addr >= NES_MEMORY_RAM_START &&
		addr <= NES_MEMORY_RAM_END)
	{
		// Read from CPU RAM.
		if (!ram_.Read16(addr - NES_MEMORY_RAM_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PRGROM_START &&
		addr <= NES_MEMORY_PRGROM_END)
	{
		// Read from PRG-ROM.
		if (!prgRom_.Read16(addr - NES_MEMORY_PRGROM_START, outVal))
			return false;
	}
	else
	{
		// @TODO Implement bus fully.
		return false;
	}

	return true;
}


bool NESCPUMemoryBus::HandleWriteMirrors(u16 addr, u8 val)
{
	if (addr >= 0x2000 && addr <= 0x2007)
	{
		// Mirror 0x2000 to 0x2007 into 0x2008 1023 (0x3FF) times.
		for (int i = 1; i <= 0x3FF; ++i)
		{
			if (!Write8(addr + (i * 8), val))
				return false;
		}
	}

	// Memory from 0x0 to 0x07FF needs to be mirrored to 0x0800, 0x1000 and 0x1800.
	if (addr <= 0x07FF)
	{
		if (!Write8(addr + 0x0800, val) ||
			!Write8(addr + 0x1000, val) ||
			!Write8(addr + 0x1800, val))
			return false;
	}

	return true;
}