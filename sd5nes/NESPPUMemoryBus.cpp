#include "NESPPUMemoryBus.h"


NESPPUMemoryBus::NESPPUMemoryBus(
	std::array<NESMemory, 2>& patternTables,
	std::array<NESMemory, 4>& nameTables,
	std::array<NESMemory, 4>& attribTables,
	NESMemory& paletteMem
	) :
	patternTables_(patternTables),
	nameTables_(nameTables),
	attribTables_(attribTables),
	paletteMem_(paletteMem)
{
}


NESPPUMemoryBus::~NESPPUMemoryBus()
{
}


bool NESPPUMemoryBus::Write8(u16 addr, u8 val)
{
	// Handle mirroring if necessary.
	if (!HandleWriteMirrors(addr, val))
		return false;

	// @TODO HACKHACK Come up with neater solution lmao.

	if (addr >= NES_MEMORY_PPU_PATTERN_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_PATTERN_TABLE_0_END)
	{
		// Write to Pattern Table 0.
		if (!patternTables_[0].Write8(addr, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_PATTERN_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_PATTERN_TABLE_1_END)
	{
		// Write to Pattern Table 1.
		if (!patternTables_[1].Write8(addr - NES_MEMORY_PPU_PATTERN_TABLE_1_START, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_0_END)
	{
		// Write to Name Table 0.
		if (!nameTables_[0].Write8(addr - NES_MEMORY_PPU_NAME_TABLE_0_START, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_1_END)
	{
		// Write to Name Table 1.
		if (!nameTables_[1].Write8(addr - NES_MEMORY_PPU_NAME_TABLE_1_START, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_2_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_2_END)
	{
		// Write to Name Table 2.
		if (!nameTables_[2].Write8(addr - NES_MEMORY_PPU_NAME_TABLE_2_START, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_3_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_3_END)
	{
		// Write to Name Table 3.
		if (!nameTables_[3].Write8(addr - NES_MEMORY_PPU_NAME_TABLE_3_START, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_END)
	{
		// Write to Attribute Table 0.
		if (!attribTables_[0].Write8(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_START, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_END)
	{
		// Write to Attribute Table 1.
		if (!attribTables_[1].Write8(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_START, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_END)
	{
		// Write to Attribute Table 2.
		if (!attribTables_[2].Write8(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_START, val))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_3_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_3_END)
	{
		// Write to Attribute Table 3.
		if (!attribTables_[3].Write8(addr - NES_MEMORY_PPU_NAME_TABLE_3_START, val))
			return false;
	}

	return true;
}


bool NESPPUMemoryBus::Read8(u16 addr, u8* outVal) const
{
	// @TODO HACKHACK Come up with neater solution lmao.

	if (addr >= NES_MEMORY_PPU_PATTERN_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_PATTERN_TABLE_0_END)
	{
		// Read from Pattern Table 0.
		if (!patternTables_[0].Read8(addr, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_PATTERN_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_PATTERN_TABLE_1_END)
	{
		// Read from Pattern Table 1.
		if (!patternTables_[1].Read8(addr - NES_MEMORY_PPU_PATTERN_TABLE_1_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_0_END)
	{
		// Read from Name Table 0.
		if (!nameTables_[0].Read8(addr - NES_MEMORY_PPU_NAME_TABLE_0_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_1_END)
	{
		// Read from Name Table 1.
		if (!nameTables_[1].Read8(addr - NES_MEMORY_PPU_NAME_TABLE_1_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_2_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_2_END)
	{
		// Read from Name Table 2.
		if (!nameTables_[2].Read8(addr - NES_MEMORY_PPU_NAME_TABLE_2_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_3_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_3_END)
	{
		// Read from Name Table 3.
		if (!nameTables_[3].Read8(addr - NES_MEMORY_PPU_NAME_TABLE_3_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_END)
	{
		// Read from Attribute Table 0.
		if (!attribTables_[0].Read8(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_END)
	{
		// Read from Attribute Table 1.
		if (!attribTables_[1].Read8(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_END)
	{
		// Read from Attribute Table 2.
		if (!attribTables_[2].Read8(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_3_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_3_END)
	{
		// Read from Attribute Table 3.
		if (!attribTables_[3].Read8(addr - NES_MEMORY_PPU_NAME_TABLE_3_START, outVal))
			return false;
	}

	return true;
}


bool NESPPUMemoryBus::Read16(u16 addr, u16* outVal) const
{
	// @TODO HACKHACK Come up with neater solution lmao.

	if (addr >= NES_MEMORY_PPU_PATTERN_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_PATTERN_TABLE_0_END)
	{
		// Read from Pattern Table 0.
		if (!patternTables_[0].Read16(addr, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_PATTERN_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_PATTERN_TABLE_1_END)
	{
		// Read from Pattern Table 1.
		if (!patternTables_[1].Read16(addr - NES_MEMORY_PPU_PATTERN_TABLE_1_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_0_END)
	{
		// Read from Name Table 0.
		if (!nameTables_[0].Read16(addr - NES_MEMORY_PPU_NAME_TABLE_0_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_1_END)
	{
		// Read from Name Table 1.
		if (!nameTables_[1].Read16(addr - NES_MEMORY_PPU_NAME_TABLE_1_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_2_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_2_END)
	{
		// Read from Name Table 2.
		if (!nameTables_[2].Read16(addr - NES_MEMORY_PPU_NAME_TABLE_2_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_NAME_TABLE_3_START &&
		addr <= NES_MEMORY_PPU_NAME_TABLE_3_END)
	{
		// Read from Name Table 3.
		if (!nameTables_[3].Read16(addr - NES_MEMORY_PPU_NAME_TABLE_3_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_END)
	{
		// Read from Attribute Table 0.
		if (!attribTables_[0].Read16(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_0_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_END)
	{
		// Read from Attribute Table 1.
		if (!attribTables_[1].Read16(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_1_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_END)
	{
		// Read from Attribute Table 2.
		if (!attribTables_[2].Read16(addr - NES_MEMORY_PPU_ATTRIBUTE_TABLE_2_START, outVal))
			return false;
	}
	else if (addr >= NES_MEMORY_PPU_ATTRIBUTE_TABLE_3_START &&
		addr <= NES_MEMORY_PPU_ATTRIBUTE_TABLE_3_END)
	{
		// Read from Attribute Table 3.
		if (!attribTables_[3].Read16(addr - NES_MEMORY_PPU_NAME_TABLE_3_START, outVal))
			return false;
	}

	return true;
}


bool NESPPUMemoryBus::HandleWriteMirrors(u16 addr, u8 val)
{
	if (addr >= 0x2000 && addr <= 0x2EFF)
	{
		// Mirror 0x2000 to 0x2EFF into 0x3000 to 0x3EFF.
		if (!Write8(addr + 0x1000, val))
			return false;
	}
	else if (addr >= 0x3F00 && addr <= 0x3F1F)
	{
		// Mirror 0x3F20 to 0x3F1F into 0x3F20 to 0x3FFF.
		if (!Write8(addr + 0x0020, val))
			return false;
	}
	
	// Memory from 0x0 to 0x3FFF needs to be mirrored into 0x4000 to 0xFFFF.
	if (addr >= 0x0000 && addr <= 0x3FFF)
	{
		if (!Write8(addr + 0x4000, val))
			return false;
	}

	return true;
}