#include "NESPPU.h"

#include "NESHelper.h"


NESPPUMemoryMapper::NESPPUMemoryMapper(NESPPUMemory& mem, NESPPUMirroringType ntMirror) :
mem_(mem),
ntMirror_(ntMirror)
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
	{
		// Consider the different nametable mirroring types that could be used here.
		switch (ntMirror_)
		{
			case NESPPUMirroringType::VERTICAL:
				return std::make_pair(&mem_.nameTables[(addr & 0x7FF) / 0x400], addr & 0x3FF);
				
			case NESPPUMirroringType::HORIZONTAL:
				return std::make_pair(&mem_.nameTables[(addr & 0xFFF) / 0x800], addr & 0x3FF);

			case NESPPUMirroringType::ONE_SCREEN:
				return std::make_pair(&mem_.nameTables[0], addr & 0x3FF);

			case NESPPUMirroringType::FOUR_SCREEN:
				// @TODO
				assert(false);
				return std::make_pair(nullptr, 0);
		}
	}
	
	// $3F00 to $3FFF
	return std::make_pair(&mem_.paletteMem, addr & 0x1F); // @TODO Mirror $3F00, $3F04, $3F08 and $3F0C to $3F10...
}


NESPPU::NESPPU(NESPPUMemoryMapper& mem, sf::Image& debug) :
mem_(mem),
debug_(debug) // @TODO Debug!!
{
	Initialize();
}


NESPPU::~NESPPU()
{
}


void NESPPU::Initialize()
{
	isEvenFrame_ = true;
	vramDataAddr_ = xNtScroll_ = yNtScroll_ = 0;

	reg_ = {}; // Zero PPU registers.
	latches_ = {}; // Zero PPU latches.
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

			target.setPixel((7 - j) + (o * 8), (i % (8 * 30)),
				col == 0 ? sf::Color::Black : NES_PPU_PALETTE_COLORS[col + colorOffset].ToSFColor());
		}

		if (i % (8 * 30) == 0 && i != 0)
			++o;
	}
}


void NESPPU::WriteRegister(NESPPURegisterType reg, u8 val)
{
	if (reg == NESPPURegisterType::UNKNOWN)
	{
		assert(false && "Unknown register type!");
		return;
	}

	// Update the internal data bus value to the value being written.
	latches_.internalDataBusVal = val;

	switch (reg)
	{
	case NESPPURegisterType::PPUCTRL:
		reg_.PPUCTRL = val;
		break;

	case NESPPURegisterType::PPUMASK:
		reg_.PPUMASK = val;
		break;

	case NESPPURegisterType::OAMADDR:
		reg_.OAMADDR = val;
		break;

	case NESPPURegisterType::OAMDATA:
		reg_.OAMDATA = val;
		break;

	case NESPPURegisterType::PPUSCROLL:
		reg_.PPUSCROLL = val;

		if (latches_.isAddressLatchOn)
			yNtScroll_ = val;
		else
			xNtScroll_ = val;

		latches_.isAddressLatchOn = !latches_.isAddressLatchOn; // Toggle state of the latch.
		break;

	case NESPPURegisterType::PPUADDR:
		reg_.PPUADDR = val;

		if (latches_.isAddressLatchOn)
			vramDataAddr_ = NESHelper::ConvertTo16((vramDataAddr_ & 0xFF00) >> 8, val);
		else
			vramDataAddr_ = NESHelper::ConvertTo16(val, vramDataAddr_ & 0xFF);

		latches_.isAddressLatchOn = !latches_.isAddressLatchOn; // Toggle state of the latch.
		break;

	case NESPPURegisterType::PPUDATA:
		reg_.PPUDATA = val;
		break;

	case NESPPURegisterType::OAMDMA:
		reg_.OAMDMA = val;
		break;
	}
}


u8 NESPPU::ReadRegister(NESPPURegisterType reg)
{
	if (reg == NESPPURegisterType::UNKNOWN)
	{
		assert(false && "Unknown register type!");
		return 0;
	}

	u8 returnVal;

	switch (reg)
	{
	case NESPPURegisterType::PPUSTATUS:
		returnVal = reg_.PPUSTATUS;
		NESHelper::ClearBit(reg_.PPUSTATUS, NES_PPU_REG_PPUSTATUS_V_BIT); // The V-Blank flag is cleared upon read.
		latches_.isAddressLatchOn = false; // Reading PPUSTATUS resets the address latch.
		break;

	case NESPPURegisterType::OAMDATA:
		returnVal = reg_.OAMDATA;
		break;

	case NESPPURegisterType::PPUDATA:
		returnVal = reg_.PPUDATA;
		break;

	default:
		returnVal = latches_.internalDataBusVal;
		break;
	}

	latches_.internalDataBusVal = returnVal;
	return returnVal;
}


void NESPPU::HandleScanline(unsigned int scanline)
{
	if (scanline < 20)
	{
		// "VINT period"
		// Do nothing.
	}
	else if (scanline < 240)
	{
		// 32 tiles per scanline.
		for (int i = 0; i < 32; ++i)
		{
			// Visible scanlines.
			const u8 ntOffset = (reg_.PPUCTRL & 3) * 0x400;
			const u8 ntByte = mem_.Read8(0x2000 + ntOffset + xNtScroll_ + (32 * yNtScroll_) + i);
			const u8 atByte = mem_.Read8(0x23C0 + ntOffset + ((xNtScroll_ + i) / 2) + (8 * (yNtScroll_ / 2))); // Attrib entry represents 2x2 grid of tiles.

			const u8 ptOffset = (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_B_BIT) ? 0x1000 : 0);
			const u8 ptHi = mem_.Read8(ptOffset + ntByte);
			const u8 ptLo = mem_.Read8(ptOffset + ntByte + 1);

			// Check that this is not the dummy scanline.
			if (scanline != 20)
			{
				for (int pix = 0; pix < 8; ++pix)
				{
					// Determine the colour of the pixel to draw in the palette. (0 - 3)
					const u8 colorNum = (((ptHi >> (7 - pix)) & 1) << 1) | ((ptLo >> (7 - pix)) & 1);

					u8 paletteNum;
					if (yNtScroll_ % 2 == 0) // towards the top
					{
						if (pix < 3) // towards the left
							paletteNum = atByte & 3;
						else // towards the right
							paletteNum = (atByte & 0xC) >> 2;
					}
					else // towards the bottom
					{
						if (pix < 3) // towards the left
							paletteNum = (atByte & 0x30) >> 4;
						else // towards the right
							paletteNum = (atByte & 0xC0) >> 6;
					}

					// Get the color to use from the table of colors.
					const auto color = NES_PPU_PALETTE_COLORS[mem_.Read8(colorNum == 0 ? 0x3F00 : 0x3F01 + (3 * paletteNum) + colorNum)];

					// @TODO DEBUG!
					debug_.setPixel((i * 8) + pix, scanline, color.ToSFColor());
				}
			}
		}
	}
	else if (scanline == 261)
	{
		// Pre-render scanline. Do nothing
	}
}


void NESPPU::Frame()
{
	isEvenFrame_ = !isEvenFrame_; // Toggle status of even frame.
	latches_.internalDataBusVal = 0; // Decay the latch.

	for (int i = 0; i < 262; ++i) // @TODO: PAL scanlines.
		HandleScanline(i);

	// @TODO
}