#include "NESMMC.h"


NESMMC::NESMMC()
{
}


NESMMC::~NESMMC()
{
}


u8 NESMMC::Read8(u16 addr) const
{
	const auto mapping = GetMMCMapping(addr);
	if (mapping.first == nullptr)
		throw NESMemoryException("Attemptted to read MMC unmapped memory!");

	return mapping.first->Read8(mapping.second);
}


NESMMCNROM::NESMMCNROM(NESMemPRGROMBank& prgRom1, NESMemPRGROMBank& prgRom2)
{
	loadedPrgRomBanks_[0] = &prgRom1;
	loadedPrgRomBanks_[1] = &prgRom2;
}


NESMMCNROM::NESMMCNROM(NESMemPRGROMBank& prgRomBank) :
NESMMCNROM(prgRomBank, prgRomBank)
{
}


NESMMCNROM::~NESMMCNROM()
{
}


std::pair<const INESMemoryInterface*, u16> NESMMCNROM::GetMMCMapping(u16 addr) const
{
	assert(addr >= 0x8000);
	return std::make_pair(loadedPrgRomBanks_[(addr - 0x8000) / 0x4000], addr & 0x3FFF);
}