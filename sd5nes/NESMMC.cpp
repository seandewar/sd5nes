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
	if (addr < 0x8000) // Cartridge SRAM
		sram_.Write8(addr - 0x6000, val);
}


u8 NESMMC::Read8(u16 addr) const
{
	if (addr < 0x8000) // Cartridge SRAM
		return sram_.Read8(addr - 0x6000);

	return 0;
}


NESMMCNROM::NESMMCNROM(NESMemSRAM& sram, NESMemCHRROMBank* chr, NESMemPRGROMBank& prg1, NESMemPRGROMBank* prg2) :
NESMMC(sram),
chr_(chr)
{
	prg_[0] = &prg1;
	prg_[1] = (prg2 != nullptr ? prg2 : &prg1);
}


NESMMCNROM::~NESMMCNROM()
{
}


void NESMMCNROM::Write8(u16 addr, u8 val)
{
	NESMMC::Write8(addr, val);
}


u8 NESMMCNROM::Read8(u16 addr) const
{
	if (addr < 0x2000) // CHR-ROM
		return (chr_ != nullptr ? chr_->Read8(addr) : 0);
	else if (addr >= 0x8000)// Upper & Lower PRG-ROM Banks
		return prg_[(addr - 0x8000) / 0x4000]->Read8(addr & 0x3FFF);
	else
		return NESMMC::Read8(addr);
}