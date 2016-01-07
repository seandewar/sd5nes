#include "NESPPUEmuComm.h"


NESPPUEmuComm::NESPPUEmuComm(NESPPUMemory& mem, NESCPU& cpu, NESPPUMirroringType ntMirror) :
mem_(mem),
cpu_(cpu),
ntMirror_(ntMirror)
{
	assert(ntMirror != NESPPUMirroringType::UNKNOWN);
}


NESPPUEmuComm::~NESPPUEmuComm()
{
}


std::size_t NESPPUEmuComm::GetNameTableIndex(u16 addr) const
{
	switch (ntMirror_)
	{
	case NESPPUMirroringType::VERTICAL:
		return (addr & 0x7FF) / 0x400;

	case NESPPUMirroringType::HORIZONTAL:
		return (addr & 0xFFF) / 0x800;

	case NESPPUMirroringType::ONE_SCREEN:
		return 0;

	case NESPPUMirroringType::FOUR_SCREEN:
		// @TODO NOT IMPLEMENTED YET!

	default:
		assert("Invalid name table mirror type!" && false);
		return NES_INVALID_NAME_TABLE_INDEX;
	}
}


void NESPPUEmuComm::Write8(u16 addr, u8 val)
{
	addr &= 0x3FFF; // Mirror ($0000 .. $3FFF) to $4000

	if (addr < 0x2000) // Pattern tables
		mem_.patternTables[addr / 0x1000].Write8(addr & 0xFFF, val);
	else if (addr < 0x3F00) // Name tables
		mem_.nameTables[GetNameTableIndex(addr)].Write8(addr & 0x3FF, val);
	else // Palette memory
		mem_.paletteMem.Write8(addr & 0x1F, val); // @TODO Mirror $3F00, $3F04, $3F08 and $3F0C!
}


u8 NESPPUEmuComm::Read8(u16 addr) const
{
	addr &= 0x3FFF; // Mirror ($0000 .. $3FFF) to $4000

	if (addr < 0x2000) // Pattern tables
		return mem_.patternTables[addr / 0x1000].Read8(addr & 0xFFF);
	else if (addr < 0x3F00) // Name tables
		return mem_.nameTables[GetNameTableIndex(addr)].Read8(addr & 0x3FF);
	else // Palette memory
		return mem_.paletteMem.Read8(addr & 0x1F); // @TODO Mirror $3F00, $3F04, $3F08 and $3F0C!
}