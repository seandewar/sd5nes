#include "NESPPU.h"

#include "NESHelper.h"

#include <iostream> // TODO: DEBUG!


NESPPU::NESPPU(sf::Image& debug) :
comm_(nullptr),
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
		const u8 pLo = comm_->Read8(i);
		const u8 pHi = comm_->Read8(i + 8);

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


void NESPPU::Initialize(INESPPUCommunicationsInterface& comm)
{
	comm_ = &comm;
}


void NESPPU::HandlePPUDATAAccess()
{
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
		std::cout << std::hex << vScroll_ << std::endl;
		comm_->Write8(vScroll_, val);
		HandlePPUDATAAccess();
		break;

	case NESPPURegisterType::OAMDMA:
		reg_.OAMDMA = val;

		// @TODO: OAMDMA
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
		returnVal = comm_->Read8(vScroll_);
		if ((vScroll_ & 0x3FFF) < 0x3F00)
		{
			const auto bufData = ppuDataBuffered_;
			ppuDataBuffered_ = returnVal;
			returnVal = bufData;
		}
		else
		{
			// Buffered data is the mirrored palette table below.
			ppuDataBuffered_ = comm_->Read8(vScroll_ - 0x1000);
		}
		
		HandlePPUDATAAccess();
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
	assert(comm_ != nullptr);

	isEvenFrame_ = true;
	xIncdThisTick_ = yIncdThisTick_ = false;
	currentCycle_ = currentScanline_ = 0;

	isNmiPulled_ = false;
	tScroll_ = vScroll_ = xScroll_ = 0;

	ppuDataBuffered_ = 0;

	ntByte_ = atByte_ = tileBitmapHi_ = tileBitmapLo_ = 0;

	latches_.internalDataBusVal = 0;
	latches_.isAddressLatchOn = false;

	reg_.PPUCTRL = reg_.PPUMASK = reg_.PPUSCROLL = reg_.PPUADDR = reg_.OAMADDR = 0;

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
	assert(comm_ != nullptr);

	isEvenFrame_ = true;
	currentCycle_ = currentScanline_ = 0;

	isNmiPulled_ = false;
	tScroll_ = vScroll_ = xScroll_ = 0;

	ppuDataBuffered_ = 0;

	ntByte_ = atByte_ = tileBitmapHi_ = tileBitmapLo_ = 0;

	latches_.isAddressLatchOn = false;

	reg_.PPUCTRL = reg_.PPUMASK = reg_.PPUSCROLL = 0;
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


void NESPPU::TickFetchTileData()
{
	switch (currentCycle_ % 8)
	{
	case 1:
		// Fetch the Name table Byte
		ntByte_ = comm_->Read8(0x2000 | (vScroll_ & 0xFFF));
		break;

	case 3:
		// Fetch the Attribute table Byte.
		atByte_ = comm_->Read8(0x23C0 | (vScroll_ & 0xC00) | ((vScroll_ >> 4) & 0x38) | ((vScroll_ >> 2) & 7));
		break;

	case 5:
	{
		// Fetch the Tile Bitmap Low Byte from Pattern table.
		const u16 addr = (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_B_BIT) ? 0x1000 : 0) + (ntByte_ * 16) +
			((vScroll_ >> 12) & 7);
		tileBitmapLo_ = comm_->Read8(addr);

		//std::cout << "l: " << std::hex << addr << ", v: " << std::hex << +tileBitmapLo_ << std::endl;
	}
	break;

	case 7:
		// Fetch the Tile Bitmap High Byte from Pattern table.
		const u16 addr = (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_B_BIT) ? 0x1000 : 0) + (ntByte_ * 16) +
			((vScroll_ >> 12) & 7) + 8;
		tileBitmapHi_ = comm_->Read8(addr);

		//std::cout << "h: " << std::hex << addr << ", v: " << std::hex << +tileBitmapHi_ << std::endl;
		break;
	}
}


void NESPPU::Tick()
{
	assert(comm_ != nullptr);

	// @TODO: Handle PAL (70 V-BLANK scanlines instead).

	// Make sure that this isn't the idle cycle.
	if (currentCycle_ != 0)
	{
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
				comm_->PullNMI();
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
				else if (IsRenderingEnabled() && currentCycle_ >= 280 && currentCycle_ <= 304)
				{
					// v: IHGF.ED CBA..... = t: IHGF.ED CBA.....
					vScroll_ = (vScroll_ & 0x41F) | (tScroll_ & 0x7BE0);
				}
			}

			/*** Visible scanlines (0 - 239) AND ***/
			/***    Pre-render scanline (261)    ***/

			if (IsRenderingEnabled())
			{
				if (currentCycle_ == 256)
					IncrementFineY();
				else if (currentCycle_ == 257)
				{
					// v: ....F.. ...EDCBA = t: ....F.. ...EDCBA
					vScroll_ = (vScroll_ & 0x7BE0) | (tScroll_ & 0x41F);
				}

				TickFetchTileData();
				// @TODO: Sprite layer

				if ((currentCycle_ <= 256 && currentCycle_ % 8 == 0) ||
					currentCycle_ == 328 || currentCycle_ == 336)
					IncrementCoarseX();

				// Sprite evaluation
				if (currentCycle_ >= 65 && currentCycle_ <= 256)
				{
					// @TODO
				}

				// Render this pixel.
				if (currentCycle_ >= 4)
				{
					// @TODO: DEBUG!!!!
					const u8 colLo = (tileBitmapLo_ >> (7 - ((currentCycle_ - 4) % 8))) & 1;
					const u8 colHi = (tileBitmapHi_ >> (7 - ((currentCycle_ - 4) % 8))) & 1;
					const u8 colNum = (colHi << 1) | colLo;

					sf::Color testC;
					if (colNum == 0)
						testC = sf::Color::Black;
					else if (colNum == 1)
						testC = sf::Color::Green;
					else if (colNum == 2)
						testC = sf::Color::Red;
					else if (colNum == 3)
						testC = sf::Color::Yellow;
					else
						testC = sf::Color::White;

					//std::cout << ": " << +ntByte_ << "," << +atByte_ << "," << +tileBitmapLo_ << "," << +tileBitmapHi_ << std::endl;

					debug_.setPixel(currentCycle_, currentScanline_,
						//(colNum == 0 ? sf::Color::Black : ppuPalette[comm_->Read8(0x3F00 + (3 * atByte_) + colNum - 1)].ToSFColor())
						//ppuPalette[comm_->Read8(0x3F00 + (3 * atByte_) + colNum - 1)].ToSFColor()
						testC
						);
				}
			}
		}
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