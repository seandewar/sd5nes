#include "NESMemoryBus.h"


NESMemoryBus::NESMemoryBus(NESMemory<NES_MEMORY_RAM_SIZE>& ram, const NESMemory<NES_MEMORY_PRGROM_SIZE>& prgRom) :
ram_(ram),
prgRom_(prgRom)
{
}


NESMemoryBus::~NESMemoryBus()
{
}


bool NESMemoryBus::Write8(u16 addr, u8 val)
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


bool NESMemoryBus::Read8(u16 addr, u8* outVal) const
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
}


bool NESMemoryBus::Read16(u16 addr, u16* outVal) const
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
}


bool NESMemoryBus::HandleWriteMirrors(u16 addr, u8 val)
{
	if (addr <= 0x07FF)
	{
		// Mirror 0x0000 to 0x07FF to 0x0800 3 times.
		for (int i = 1; i <= 3; ++i)
		{
			if (!Write8(addr + (i * 0x0800), val))
				return false;
		}
	}
	else if (addr >= 0x2000 && addr <= 0x2007)
	{
		// Mirror 0x2000 to 0x2007 to 0x2008 to 0x4020
		for (int i = 1; i <= 0x402; ++i)
		{
			if (!Write8(addr + (i * 8), val))
				return false;
		}
	}

	return true;
}