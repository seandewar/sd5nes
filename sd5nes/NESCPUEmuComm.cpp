#include "NESCPUEmuComm.h"

#include "NESController.h"


NESCPUEmuComm::NESCPUEmuComm(NESMemCPURAM& ram, NESPPU& ppu, NESMMC& mmc, 
	const NESControllerPorts& controllers) :
ram_(ram),
ppu_(ppu),
mmc_(mmc),
controllers_(controllers)
{
}


NESCPUEmuComm::~NESCPUEmuComm()
{
}


NESPPURegisterType NESCPUEmuComm::GetPPURegister(u16 realAddr)
{
	switch (realAddr)
	{
	case 0x2000:
		return NESPPURegisterType::PPUCTRL;

	case 0x2001:
		return NESPPURegisterType::PPUMASK;

	case 0x2002:
		return NESPPURegisterType::PPUSTATUS;

	case 0x2003:
		return NESPPURegisterType::OAMADDR;

	case 0x2004:
		return NESPPURegisterType::OAMDATA;

	case 0x2005:
		return NESPPURegisterType::PPUSCROLL;

	case 0x2006:
		return NESPPURegisterType::PPUADDR;

	case 0x2007:
		return NESPPURegisterType::PPUDATA;

	case 0x4014:
		return NESPPURegisterType::OAMDMA;

	default:
		return NESPPURegisterType::UNKNOWN;
	}
}


void NESCPUEmuComm::Write8(u16 addr, u8 val)
{
	if (addr < 0x2000) // RAM
		ram_.Write8(addr & 0x7FF, val);
	else if (addr < 0x4000) // PPU I/O Registers
		ppu_.WriteRegister(GetPPURegister(0x2000 + (addr & 7)), val);
	else if (addr == 0x4014) // PPU I/O OAMDATA Register
		ppu_.WriteRegister(GetPPURegister(0x4014), val);
	else if (addr < 0x4016) // pAPU I/O Registers
		return; // @TODO
	else if (addr == 0x4016) // Controller Strobe
	{
		// Loop through active controllers and write new strobe to each one.
		for (auto controller : controllers_)
		{
			if (controller != nullptr)
				controller->WriteController(val);
		}
	}
	else if (addr == 0x4017) // pAPU Frame Counter
		return; // @TODO
	else // Use the MMC
		mmc_.Write8(addr, val);
}


u8 NESCPUEmuComm::Read8(u16 addr) const
{
	if (addr < 0x2000) // RAM
		return ram_.Read8(addr & 0x7FF);
	else if (addr < 0x4000) // PPU I/O Registers
		return ppu_.ReadRegister(GetPPURegister(0x2000 + (addr & 7)));
	else if (addr == 0x4014) // PPU I/O OAMDATA Register
		return ppu_.ReadRegister(GetPPURegister(0x4014));
	else if (addr < 0x4016) // pAPU I/O Registers
		return 0; // @TODO
	else if (addr < 0x4018) // Controllers 1 and 2
	{
		auto controller = controllers_[addr - 0x4016];
		return (controller != nullptr ? controller->ReadController() : 0);
	}
	else // Use the MMC
		return mmc_.Read8(addr);
}
