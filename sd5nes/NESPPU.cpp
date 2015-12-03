#include "NESPPU.h"

#include "NESHelper.h"


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
	Initialize();
}


NESPPU::~NESPPU()
{
}


void NESPPU::Initialize()
{
	isEvenFrame_ = true;
	currentScanline_ = 0;
}


void NESPPU::DebugDrawPatterns(sf::Image& target, int colorOffset)
{
	target.create(240, 248, sf::Color::Black);

	int o = 0;
	// Loop through the pattern tables.
	for (int i = 0; i < 0x2000; ++i)
	{
		const u8 pLo = mem_.Read8(i);
		const u8 pHi = mem_.Read8(i + 8);

		for (int j = 7; j >= 0; --j)
		{
			// Get the color.
			const u8 colHi = (pHi >> j) & 1;
			const u8 colLo = (pLo >> j) & 1;
			const auto col = (colHi << 1) | colLo;

			target.setPixel((7 - j) + (o * 8), (i % (8 * 30)), col == 0 ? sf::Color::Black : NES_PPU_PALETTE_COLORS[col + colorOffset].ToSFColor());
		}

		if (i % (8 * 30) == 0 && i != 0)
			++o;
	}
}


void NESPPU::HandleScanline()
{
	// @TODO: Pre-render scanline
	if (currentScanline_ >= 0 && currentScanline_ <= 239)
	{
		// Visible scanline.
		// @TODO Idle cycle.

		// ...
	}
}


void NESPPU::Tick()
{
	isEvenFrame_ = !isEvenFrame_; // Toggle status of even frame.

	HandleScanline();
	++currentScanline_;

	// @TODO
}


NESPPURegisters& NESPPU::GetPPURegisters()
{
	return ppuReg_;
}
