#include "NESPPUEmuComm.h"


NESPPUEmuComm::NESPPUEmuComm(NESPPUMemory& mem, NESCPU& cpu, INESMMC& mmc, NESNameTableMirroringType ntMirror) :
mem_(mem),
cpu_(cpu),
mmc_(&mmc),
ntMirror_(ntMirror)
{
	assert(ntMirror != NESNameTableMirroringType::UNKNOWN);
}


NESPPUEmuComm::~NESPPUEmuComm()
{
}


std::size_t NESPPUEmuComm::GetNameTableIndex(u16 addr) const
{
	switch (ntMirror_)
	{
	case NESNameTableMirroringType::VERTICAL:
		return (addr & 0x7FF) / 0x400;

	case NESNameTableMirroringType::HORIZONTAL:
		return (addr & 0xFFF) / 0x800;

	case NESNameTableMirroringType::ONE_SCREEN:
		return 0;

	case NESNameTableMirroringType::FOUR_SCREEN:
		// @TODO NOT IMPLEMENTED YET!

	default:
		assert("Invalid name table mirror type!" && false);
		return NES_INVALID_NAME_TABLE_INDEX;
	}
}


std::array<u8, 0x100> NESPPUEmuComm::OAMDMARead(u8 addrPage)
{
	// DMA causes CPU stall for 513 cycles (+1 on odd CPU cycle).
	cpu_.StallFor(513 + (cpu_.GetElapsedCycles() % 2 == 1 ? 1 : 0));

	// DMA CPU Memory in order to read 256 bytes from page.
	const u16 addrStart = addrPage << 8;

	std::array<u8, 0x100> retMem;
	for (std::size_t i = 0; i < retMem.size(); ++i)
		retMem[i] = cpu_.ReadMemory8(addrStart + i);

	return retMem;
}


void NESPPUEmuComm::Write8(u16 addr, u8 val)
{
	addr &= 0x3FFF; // Mirror ($0000 .. $3FFF) to $4000

	if (addr < 0x2000) // Pattern tables
		mmc_->Write8(addr, val);
	else if (addr < 0x3F00) // Name tables
		mem_.nameTables[GetNameTableIndex(addr)].Write8(addr & 0x3FF, val);
	else // Palette memory
	{
		mem_.paletteMem.Write8(addr & 0x1F, val);

		// Mirror $3F00, $3F04, $3F08, $3F0C to $3F10, $3F14, $3F18, $3F1C.
		if ((addr & 3) == 0)
			mem_.paletteMem.Write8((addr & 0x1F) ^ 0x10, val);
	}
}


u8 NESPPUEmuComm::Read8(u16 addr) const
{
	addr &= 0x3FFF; // Mirror ($0000 .. $3FFF) to $4000

	if (addr < 0x2000) // Pattern tables
		return mmc_->Read8(addr);
	else if (addr < 0x3F00) // Name tables
		return mem_.nameTables[GetNameTableIndex(addr)].Read8(addr & 0x3FF);
	else // Palette memory
		return mem_.paletteMem.Read8(addr & 0x1F);
}