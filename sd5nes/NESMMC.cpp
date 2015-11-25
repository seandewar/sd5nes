#include "NESMMC.h"


NESMMC::NESMMC()
{
}


NESMMC::~NESMMC()
{
}


NESMMCNROM::NESMMCNROM(const std::array<std::reference_wrapper<NESMemPRGROMBank>, 2>& prgRomBanks)
{
	loadedPrgRomBanks_.assign(prgRomBanks.begin(), prgRomBanks.end());
}

NESMMCNROM::NESMMCNROM(NESMemPRGROMBank& prgRomBank) :
NESMMCNROM(std::array<std::reference_wrapper<NESMemPRGROMBank>, 2> { {prgRomBank, prgRomBank} })
{
}

NESMMCNROM::~NESMMCNROM()
{
}


std::pair<INESMemoryInterface*, u16> NESMMCNROM::GetMapping(u16 addr) const
{
	if (addr >= 0x8000)
		return std::make_pair(&loadedPrgRomBanks_[(addr - 0x8000) / 0x4000].get(), addr & 0x3FFF);
}