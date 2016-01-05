#include "NESPPU.h"

#include "NESHelper.h"


NESPPUMemoryMapper::NESPPUMemoryMapper(NESPPUMemory& mem, NESPPUMirroringType ntMirror) :
mem_(mem),
ntMirror_(ntMirror)
{
	assert(ntMirror != NESPPUMirroringType::UNKNOWN);
}


NESPPUMemoryMapper::~NESPPUMemoryMapper()
{
}


std::size_t NESPPUMemoryMapper::GetNameTableIndex(u16 addr) const
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


void NESPPUMemoryMapper::Write8(u16 addr, u8 val)
{
	addr &= 0x3FFF; // Mirror ($0000 .. $3FFF) to $4000

	if (addr < 0x2000) // Pattern tables
		mem_.patternTables[addr / 0x1000].Write8(addr & 0xFFF, val);
	else if (addr < 0x3F00) // Name tables
		mem_.nameTables[GetNameTableIndex(addr)].Write8(addr & 0x3FF, val);
	else // Palette memory
		mem_.paletteMem.Write8(addr & 0x1F, val); // @TODO Mirror $3F00, $3F04, $3F08 and $3F0C!
}


u8 NESPPUMemoryMapper::Read8(u16 addr) const
{
	addr &= 0x3FFF; // Mirror ($0000 .. $3FFF) to $4000

	if (addr < 0x2000) // Pattern tables
		return mem_.patternTables[addr / 0x1000].Read8(addr & 0xFFF);
	else if (addr < 0x3F00) // Name tables
		return mem_.nameTables[GetNameTableIndex(addr)].Read8(addr & 0x3FF);
	else // Palette memory
		return mem_.paletteMem.Read8(addr & 0x1F); // @TODO Mirror $3F00, $3F04, $3F08 and $3F0C!
}


NESPPU::NESPPU(NESPPUMemoryMapper& mem, NESCPU& cpu, sf::Image& debug) :
mem_(mem),
cpu_(cpu),
isNmiPulled_(false),
isEvenFrame_(true),
xIncdThisTick_(false),
yIncdThisTick_(false),
vScroll_(0),
tScroll_(0),
xScroll_(0),
currentCycle_(0),
elapsedCycles_(0),
debug_(debug) // @TODO Debug!!
{
}


NESPPU::~NESPPU()
{
}


void NESPPU::DebugDrawPatterns(sf::Image& target, int colorOffset)
{
	target.create(240, 248, sf::Color::Black);

	int o = 0;
	// Loop through the pattern tables.
	for (std::size_t i = 0; i < 0x2000; ++i)
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
				col == 0 ? sf::Color::Black : ppuPalette[col + colorOffset].ToSFColor());
		}

		if (i % (8 * 30) == 0 && i != 0)
			++o;
	}
}


void NESPPU::WriteRegister(NESPPURegisterType reg, u8 val)
{
	// Update the internal data bus value to the value being written.
	latches_.internalDataBusVal = val;

	switch (reg)
	{
	case NESPPURegisterType::UNKNOWN:
		assert(false && "Unknown register type!");
		return;

	case NESPPURegisterType::PPUCTRL:
		reg_.PPUCTRL = val;

		// Reset the Nmi Pull if we wrote 0 to V in PPUCTRL.
		// This will allow multiple NMIs to be generated if toggled
		// during V-BLANK.
		if (!NESHelper::IsBitSet(val, NES_PPU_REG_PPUCTRL_V_BIT))
			isNmiPulled_ = false;

		// t: ...BA.. ........ = d: ......BA
		tScroll_ = (tScroll_ & 0x73FF) | ((val & 3) << 10);
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
		{
			// t: CBA..HG FED..... = d : HGFEDCBA
			tScroll_ = (((tScroll_ & 0x7C1F) | ((val & 0xF8) << 2)) & 0xFFF) | ((val & 7) << 12);
		}
		else
		{
			// t: ....... ...HGFED = d: HGFED...
			tScroll_ = (tScroll_ & 0x7FE0) | ((val & 0xF8) >> 3);

			// x:              CBA = d: .....CBA
			xScroll_ = val & 7;
		}

		latches_.isAddressLatchOn = !latches_.isAddressLatchOn; // Toggle state of the latch.
		break;

	case NESPPURegisterType::PPUADDR:
		reg_.PPUADDR = val;

		if (latches_.isAddressLatchOn)
		{
			// t: ....... HGFEDCBA = d: HGFEDCBA
			// v                   = t
			vScroll_ = tScroll_ = (tScroll_ & 0x7F00) | (val & 0xFF);
		}
		else
		{
			// t: .FEDCBA ........ = d: ..FEDCBA
			// t: X...... ........ = 0
			tScroll_ = (tScroll_ & 0xFF) | ((val & 0x3F) << 8);
		}

		latches_.isAddressLatchOn = !latches_.isAddressLatchOn; // Toggle state of the latch.
		break;

	case NESPPURegisterType::PPUDATA:
		reg_.PPUDATA = val;

		// @NOTE: PPU has some strange behaviour where it increments both coarse X and fine Y
		// if rendering is enabled and the PPU is currently handling
		// the pre-render or visible scanlines.
		if ((currentScanline_ <= 239 || currentScanline_ == 261) &&
			IsRenderingEnabled())
		{
			IncrementCoarseX();
			IncrementFineY();
		}
		else
		{
			// Increment v by 32 if I in PPUCTRL is set, otherwise by 1 instead.
			if (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_I_BIT))
				vScroll_ += 32;
			else
				vScroll_ += 1;
		}
		break;

	case NESPPURegisterType::OAMDMA:
		reg_.OAMDMA = val;
		break;
	}
}


u8 NESPPU::ReadRegister(NESPPURegisterType reg)
{
	u8 returnVal;

	switch (reg)
	{
	case NESPPURegisterType::UNKNOWN:
		assert(false && "Unknown register type!");
		return 0;

	case NESPPURegisterType::PPUSTATUS:
		returnVal = reg_.PPUSTATUS;

		// The V-Blank flag is cleared upon read
		NESHelper::ClearRefBit(reg_.PPUSTATUS, NES_PPU_REG_PPUSTATUS_V_BIT);

		latches_.isAddressLatchOn = false; // Reset address latch
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


void NESPPU::Power()
{
	isEvenFrame_ = true;
	xIncdThisTick_ = yIncdThisTick_ = false;
	currentCycle_ = currentScanline_ = 0;

	isNmiPulled_ = false;
	tScroll_ = vScroll_ = xScroll_ = 0;

	latches_.internalDataBusVal = 0;
	latches_.isAddressLatchOn = false;

	reg_.PPUCTRL = reg_.PPUMASK = reg_.PPUSCROLL = reg_.PPUADDR 
		= reg_.PPUDATA = reg_.OAMADDR = 0;

	// Set up PPUSTATUS Power state - depends on a few random variables
	// O and V are often set in PPUSTATUS
	// Other bits are irrelevant (except S which should be 0).
	reg_.PPUSTATUS = 0;
	NESHelper::EditRefBit(reg_.PPUSTATUS, NES_PPU_REG_PPUSTATUS_V_BIT, 
		NESHelper::GetRandomBool(NES_PPU_POWER_REG_PPUSTATUS_V_SET_CHANCE));
	NESHelper::EditRefBit(reg_.PPUSTATUS, NES_PPU_REG_PPUSTATUS_O_BIT, 
		NESHelper::GetRandomBool(NES_PPU_POWER_REG_PPUSTATUS_O_SET_CHANCE));
	
	// @TODO: Init NT RAM to mostly $FF and CHR RAM to unspec pattern and
	// OAM to pattern
}


void NESPPU::Reset()
{
	isEvenFrame_ = true;
	currentCycle_ = currentScanline_ = 0;

	isNmiPulled_ = false;
	tScroll_ = vScroll_ = xScroll_ = 0;

	latches_.isAddressLatchOn = false;

	reg_.PPUCTRL = reg_.PPUMASK = reg_.PPUSCROLL = reg_.PPUDATA = 0;
	reg_.PPUSTATUS &= 0x80; // Only retain bit 7. (PPUSTATUS V)

	// @TODO: Init OAM to pattern
}


void NESPPU::IncrementCoarseX()
{
	// Do not double increment for this tick.
	if (xIncdThisTick_)
		return;

	xIncdThisTick_ = true;

	// Coarse X is bits 0-4 in v.
	if ((vScroll_ & 0x1F) == 0x1F)
	{
		// Coarse X is currently 31 (0x1F) which is its max value.
		// So we need to clear coarse X and switch horiz nt.
		vScroll_ = (vScroll_ & 0x7FE0) ^ 0x400;
	}
	else
	{
		// No wrap needed, just increment coarse X as it was below 31.
		vScroll_ += 1;
	}
}


void NESPPU::IncrementFineY()
{
	// Do not double increment for this tick.
	if (yIncdThisTick_)
		return;

	yIncdThisTick_ = true;

	// Fine Y scroll is bits 12-14 in v.
	if ((vScroll_ & 0x7000) == 0x7000)
	{
		// Fine Y is currently 7 which means we need to check
		// coarse Y (bits 5-9 in v).
		vScroll_ &= 0xFFF; // Clear fine y scroll.

		const u8 coarseY = (vScroll_ & 0x3E0) >> 5;
		if (coarseY == 0x1D) // == 29
		{
			// Switch vertical nt by toggling bit 11
			// and clear coarse Y.
			vScroll_ = (vScroll_ & 0x7C1F) ^ 0x800;
		}
		else if (coarseY == 0x1F) // == 31 (max value)
		{
			// Clear coarse Y.
			vScroll_ &= 0x7C1F;
		}
		else
		{
			// Increment coarse Y.
			vScroll_ = (vScroll_ & 0x7C1F) | (((coarseY + 1) & 0xF) << 5);
		}
	}
	else
	{
		// No wrap needed, we can just increment fine Y as it was below 7.
		vScroll_ += 0x1000;
	}
}


void NESPPU::Tick()
{
	// @TODO: Handle PAL (70 V-BLANK scanlines instead).

	if (currentScanline_ >= 240 && currentScanline_ <= 260)
	{
		/*** Post-render (idle) scanline (240) OR ***/
		/*** V-BLANK period (241 - 260)           ***/

		// Set V-BLANK on cycle 1 of scanline 241.
		if (currentScanline_ == 241 && currentCycle_ == 1)
		{
			// Set V flag in PPUSTATUS and make sure Nmi isn't pulled.
			NESHelper::SetRefBit(reg_.PPUSTATUS, NES_PPU_REG_PPUSTATUS_V_BIT);
			isNmiPulled_ = false;
		}

		if (currentScanline_ >= 241 &&
			NESHelper::IsBitSet(reg_.PPUSTATUS, NES_PPU_REG_PPUSTATUS_V_BIT) &&
			NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_V_BIT) &&
			!isNmiPulled_)
		{
			// We should set the V-BLANK NMI now.
			isNmiPulled_ = true;
			cpu_.SetInterrupt(NESCPUInterruptType::NMI); // @TODO Don't reference CPU directly..?
		}
	}
	else
	{
		if (currentScanline_ == 261)
		{
			/*** Pre-render scanline (261) ***/

			if (currentCycle_ == 1)
			{
				// Clear PPUSTATUS flags on cycle 1.
				reg_.PPUSTATUS = 0;
			}
			else if (currentCycle_ >= 280 && currentCycle_ < 305 && IsRenderingEnabled())
			{
				// v: IHGF.ED CBA..... = t: IHGF.ED CBA.....
				vScroll_ = (vScroll_ & 0x41F) | (tScroll_ & 0x7BE0);
			}
		}
		//else
		//{
		//	/*** Visible scanlines (0 - 239) ***/
		//}

		if (IsRenderingEnabled())
		{
			if (currentCycle_ == 256)
				IncrementFineY();
			else if (currentCycle_ == 257)
			{
				// v: ....F.. ...EDCBA = t: ....F.. ...EDCBA
				vScroll_ = (vScroll_ & 0x7BE0) | (tScroll_ & 0x41F);
			}
		}

		if ((currentCycle_ != 0 && currentCycle_ <= 256 && currentCycle_ % 8 == 0) ||
			currentCycle_ == 328 || currentCycle_ == 336)
			IncrementCoarseX();
	}

	++elapsedCycles_;
	++currentCycle_;
	xIncdThisTick_ = yIncdThisTick_ = false;

	// Check if we're at the end of this scanline (which is one cycle earlier (339)
	// on the pre-render scanline (261) when on an odd frame).
	if (currentCycle_ > 340 || 
		(currentScanline_ == 261 && currentCycle_ == 339 && !isEvenFrame_))
	{
		currentCycle_ = 0;
		++currentScanline_;

		// Check if we've finished handling the entire frame.
		if (currentScanline_ > 261)
		{
			currentScanline_ = 0;
			isEvenFrame_ = !isEvenFrame_;
		}
	}
}