#include "NESPPU.h"

NESPPUMemoryMapper::NESPPUMemoryMapper(NESPPUMemory& mem) :
mem_(mem)
{
}


NESPPUMemoryMapper::~NESPPUMemoryMapper()
{
}


std::pair<INESMemoryInterface*, u16> NESPPUMemoryMapper::GetMapping(u16 addr) const
{
	addr &= 0x3FFF; // Mirror $0000 - $3FFF to $4000

	if (addr < 0x2000)
		return std::make_pair(&mem_.patternTables[addr / 0x1000], addr & 0xFFF);
	else if (addr < 0x3F00)
		return std::make_pair(&mem_.nameTables[((addr - 0x2000) & 0xFFF) / 0x400], addr & 0x3FF);
	else // $3F00 to $3FFF
		return std::make_pair(&mem_.paletteMem, addr & 0x1F);
}


NESPPU::NESPPU(NESPPUMemoryMapper& mem) :
mem_(mem)
{
}


NESPPU::~NESPPU()
{
}


NESPPURegisters& NESPPU::GetPPURegisters()
{
	return ppuReg_;
}
