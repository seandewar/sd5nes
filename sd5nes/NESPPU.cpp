#include "NESPPU.h"

#include <iostream> // TODO: DEBUG!


NESPPU::NESPPU(sf::Image& debug) :
comm_(nullptr),
isFrameFinished_(false),
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
	// @NOTE: PPU has some strange behaviour where it increments both X and Y of v
	// if rendering is enabled and the PPU is currently handling
	// the pre-render or visible scanlines.
	if ((currentScanline_ <= 239 || currentScanline_ == 261) &&
		IsRenderingEnabled())
	{
		IncrementScrollX();
		IncrementScrollY();
	}
	else
	{
		// Increment v by 32 if I in PPUCTRL is set, otherwise by 1 instead.
		vScroll_ += (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_I_BIT) ? 32 : 1);
	}
}


void NESPPU::WriteRegister(NESPPURegisterType reg, u8 val)
{
	// Update the internal data bus value to the value being written.
	latches_.internalDataBusVal = val;
	latches_.cyclesLeftUntilBusDecay = NES_PPU_DATA_BUS_DECAY_CYCLES;

	switch (reg)
	{
	case NESPPURegisterType::UNKNOWN:
		assert(false && "Unknown register type!");
		return;

		// Check if we should ignore writes to some registers.
		if (reg_.writeIgnoreCyclesLeft == 0)
		{
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

		case NESPPURegisterType::PPUSCROLL:
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
		}

	case NESPPURegisterType::PPUDATA:
		comm_->Write8(vScroll_, val);
		HandlePPUDATAAccess();
		break;

	case NESPPURegisterType::OAMADDR:
		reg_.OAMADDR = val;
		break;

	case NESPPURegisterType::OAMDATA:
		primaryOam_.Write8(reg_.OAMADDR, val);
		++reg_.OAMADDR;
		break;

	case NESPPURegisterType::OAMDMA:
		const auto oamDmaData = comm_->OAMDMARead(val);
		for (u16 i = 0; i < 0x100 - reg_.OAMADDR; ++i)
			primaryOam_.Write8((i + reg_.OAMADDR) & 0xFF, oamDmaData[i]);
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
		// @TODO: Handle 0xFF Signal for clearing secondary OAM
		returnVal = primaryOam_.Read8(reg_.OAMADDR);
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

	isFrameFinished_ = false;
	isEvenFrame_ = true;

	xIncdThisTick_ = yIncdThisTick_ = false;
	currentCycle_ = currentScanline_ = 0;

	isNmiPulled_ = false;
	tScroll_ = vScroll_ = xScroll_ = activeSpriteCount_ = 0;

	ppuDataBuffered_ = 0;

	ntByte_ = atByte_ = tileBitmapHi_ = tileBitmapLo_ = 0;

	latches_.internalDataBusVal = 0;
	latches_.isAddressLatchOn = false;

	reg_.PPUCTRL = reg_.PPUMASK = reg_.OAMADDR = 0;

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

	isFrameFinished_ = false;
	isEvenFrame_ = true;

	currentCycle_ = currentScanline_ = 0;

	isNmiPulled_ = false;
	tScroll_ = vScroll_ = xScroll_ = activeSpriteCount_ = 0;

	ppuDataBuffered_ = 0;

	ntByte_ = atByte_ = tileBitmapHi_ = tileBitmapLo_ = 0;

	latches_.isAddressLatchOn = false;

	reg_.PPUCTRL = reg_.PPUMASK = 0;
	reg_.PPUSTATUS &= 0x80; // Only retain bit 7. (PPUSTATUS V)

	reg_.writeIgnoreCyclesLeft = NES_PPU_RESET_REG_IGNORE_WRITE_FOR_CPU_CYC;

	// @TODO: Init OAM to pattern
}


void NESPPU::IncrementScrollX()
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


void NESPPU::IncrementScrollY()
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
		const u8 coarseY = (vScroll_ & 0x3E0) >> 5;
		if (coarseY == 29)
		{
			// Switch vertical nt by toggling bit 11
			// and clear fine Y, coarse Y.
			vScroll_ = (vScroll_ & 0xC1F) ^ 0x800;
		}
		else if (coarseY == 0x1F) // == 31 (max value)
		{
			// Clear fine Y, coarse Y.
			vScroll_ &= 0xC1F;
		}
		else
		{
			// Clear fine Y, increment coarse Y.
			vScroll_ = (vScroll_ & 0xC1F) | (((coarseY + 1) & 0x1F) << 5);
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
	// Only evaluate tile data on visible scanlines.
	if (currentScanline_ > 239)
		return;

	switch (currentCycle_ % 8)
	{
	case 1:
		// Fetch the Name table Byte.
		ntByte_ = comm_->Read8(0x2000 | (vScroll_ & 0xFFF));
		break;

	case 3:
		// Fetch the Attribute table Byte.
		atByte_ = comm_->Read8(0x23C0 | (vScroll_ & 0xC00) | ((vScroll_ >> 4) & 0x38) | ((vScroll_ >> 2) & 7));
		break;

	case 5:
		// Fetch the Tile Bitmap Low Byte from Pattern table.
		tileBitmapLo_ = comm_->Read8(
			(NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_B_BIT) ? 0x1000 : 0) + 
			(ntByte_ * 16) + ((vScroll_ >> 12) & 7)
		);
		break;

	case 7:
		// Fetch the Tile Bitmap High Byte from Pattern table.
		tileBitmapHi_ = comm_->Read8(
			(NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_B_BIT) ? 0x1000 : 0) + 
			(ntByte_ * 16) + ((vScroll_ >> 12) & 7) + 8
		);
		break;
	}
}


void NESPPU::TickEvaluateSprites()
{
	// Only evaluate sprites during visible scanlines.
	if (currentScanline_ > 239)
		return;

	if (currentCycle_ >= 1 && currentCycle_ <= 64 && currentCycle_ % 2 == 1)
	{
		if (currentCycle_ == 1)
			activeSpriteCount_ = 0;

		// Clear secondary OAM value every 2 cycles to $FF.
		secondaryOam_.Write8((currentCycle_ - 1) / 2, 0xFF);
	}
	else if (currentCycle_ == 256) // @TODO: Cycle accuracy? CPU isn't truly cycle accurate anyway...
	{
		// Eval all 64 entries in OAM and try to find up to 8 sprites to render
		// for the next scanline.
		u8 n = 0;
		u8 m = 0;
		while (n < 64)
		{
			const u16 oamAddr = (4 * n) + m;
			const u8 sprY = primaryOam_.Read8(oamAddr);
			const bool sprInRange = (sprY <= currentScanline_ &&
				sprY + (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_H_BIT) ? 16u : 8u) > currentScanline_);

			// Check if we already have 8 sprites found and check for overflow if we do.
			// Otherwise, check if sprite is in range. If H is set in PPUCTRL, sprite is 16 px high.
			if (activeSpriteCount_ == 8)
			{
				if (sprInRange)
				{
					// Set overflow and stop here.
					NESHelper::SetRefBit(reg_.PPUSTATUS, NES_PPU_REG_PPUSTATUS_O_BIT);
					break;
				}
				else
				{
					// @NOTE: The m increment is a hardware bug and emulates the
					// bug where the O flag in PPUSTATUS is sdmetimes not set.
					++m;
					if (m > 3) // Make sure m doesn't increment above 3.
						m = 0;
				}
			}
			else if (sprInRange)
			{
				// Sprite is in range, copy to secondary OAM.
				secondaryOam_.Write8(activeSpriteCount_, sprY);
				for (u8 i = 1; i <= 3; ++i)
					secondaryOam_.Write8(activeSpriteCount_ + i, primaryOam_.Read8(oamAddr + i));

				++activeSpriteCount_;
			}

			++n;
		}

		// @NOTE: Secondary OAM always ends with Sprite 63's Y-position
		// if it isn't already full (or before the $FFs from the init).
		if (activeSpriteCount_ < 8)
			secondaryOam_.Write8(activeSpriteCount_ * 4, primaryOam_.Read8(0xFC));
	}
	else if (currentCycle_ >= 257 && currentCycle_ <= 320)
	{
		// Fetch sprites to render from secondary OAM.
		// for the next scanline.
		const u8 spriteIndex = (currentCycle_ - 257) / 8;
		const u8 sprAddr = spriteIndex * 4;
		switch ((currentCycle_ - 257) % 8)
		{
		case 0:
			activeSprites_[spriteIndex].y = secondaryOam_.Read8(sprAddr);
			break;

		case 1:
			activeSprites_[spriteIndex].tileIndex = secondaryOam_.Read8(sprAddr + 1);
			break;

		case 2:
			activeSprites_[spriteIndex].attributes = secondaryOam_.Read8(sprAddr + 2);
			break;

		case 3:
			activeSprites_[spriteIndex].x = secondaryOam_.Read8(sprAddr + 3);
			break;

		// Fetch tile bitmap of sprite (probably 2 cycles per memory access).
		case 5:
			activeSprites_[spriteIndex].tileBitmapLo = comm_->Read8(
				GetSpriteTileAddress(activeSprites_[spriteIndex].tileIndex)
			);
			break;

		case 7:
			activeSprites_[spriteIndex].tileBitmapLo = comm_->Read8(
				GetSpriteTileAddress(activeSprites_[spriteIndex].tileIndex) + 8
			);
			break;
		}
	}
}


void NESPPU::TickRenderPixel()
{
	if (currentCycle_ < 4)
		return;

	// @TODO: DEBUG!!!!
	const u8 colLo = (tileBitmapLo_ >> (7 - (currentCycle_ % 8))) & 1;
	const u8 colHi = (tileBitmapHi_ >> (7 - (currentCycle_ % 8))) & 1;
	const u8 colNum = (colHi << 1) | colLo;

	const unsigned int drawX = currentCycle_;
	const unsigned int drawY = currentScanline_;

	debug_.setPixel(drawX, drawY,
		(colNum == 0 ? sf::Color::Black : ppuPalette[comm_->Read8(0x3F00 + (3 * atByte_) + colNum - 1)].ToSFColor())
	);

	for (u8 i = 0; i < activeSpriteCount_; ++i)
	{
		if (activeSprites_[i].x == drawX)
		{
			debug_.setPixel(drawX, drawY, sf::Color::Green);
			break;
		}
	}
}


void NESPPU::Tick()
{
	assert(comm_ != nullptr);

	isFrameFinished_ = false; // We will set this true if this is the last tick of the frame.

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
					// Clear PPUSTATUS flags on cycle 1 and reset
					// internal v-blank flag.
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
					IncrementScrollY();
				else if (currentCycle_ == 257)
				{
					// v: ....F.. ...EDCBA = t: ....F.. ...EDCBA
					vScroll_ = (vScroll_ & 0x7BE0) | (tScroll_ & 0x41F);
				}

				if ((currentCycle_ <= 256 && currentCycle_ % 8 == 0) ||
					currentCycle_ == 328 || currentCycle_ == 336)
					IncrementScrollX();

				TickEvaluateSprites();
				TickFetchTileData();

				TickRenderPixel();
			}
		}
	}

	++elapsedCycles_;
	++currentCycle_;

	// Decay the value inside of the internal data bus.
	if (latches_.cyclesLeftUntilBusDecay > 0)
		--latches_.cyclesLeftUntilBusDecay;
	else
		latches_.internalDataBusVal = 0;

	// Update amount of cycles left to ignore writes to some registers.
	if (reg_.writeIgnoreCyclesLeft > 0)
		--reg_.writeIgnoreCyclesLeft;

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
			isFrameFinished_ = true;
		}
	}
}