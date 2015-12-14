#include "NESMMC.h"


NESMMC::NESMMC(NESMemSRAM& sram) :
sram_(sram)
{
}


NESMMC::~NESMMC()
{
}


void NESMMC::Write8(u16 addr, u8 val)
{
	assert(addr >= 0x6000);

	if (addr < 0x8000) // Cartridge SRAM
		sram_.Write8(addr - 0x6000, val);
}


u8 NESMMC::Read8(u16 addr) const
{
	assert(addr >= 0x6000);

	if (addr < 0x8000) // Cartridge SRAM
		return sram_.Read8(addr - 0x6000);
	else // Upper & Lower PRG-ROM Banks
		return loadedPrgRomBanks_[(addr - 0x8000) / 0x4000]->Read8(addr & 0x3FFF);
}


NESMMCNROM::NESMMCNROM(NESMemSRAM& sram, NESMemPRGROMBank& prgRom1, NESMemPRGROMBank& prgRom2) :
NESMMC(sram)
{
	loadedPrgRomBanks_[0] = &prgRom1;
	loadedPrgRomBanks_[1] = &prgRom2;
}


NESMMCNROM::NESMMCNROM(NESMemSRAM& sram, NESMemPRGROMBank& prgRomBank) :
NESMMCNROM(sram, prgRomBank, prgRomBank)
{
}


NESMMCNROM::~NESMMCNROM()
{
}