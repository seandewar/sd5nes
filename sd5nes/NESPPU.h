#pragma once

#include "NESPPUMemoryBus.h"
#include "NESPPUColor.h"

/**
* Struct containing the registers used by the NES PPU.
*/
struct NESPPURegisters
{
	union
	{
		/* PPU Control Register 1 */
		u8 control1;

		u8 nameTableNum : 2; /* Name table number. */
		u8 incVramAddr : 1; /* Increments VRAM address by 1 if this is 0, otherwise by 32 if this is 1. */
		u8 spritePatternTableNum : 1; /* Number of the Pattern table that the sprites are stored in. */
		u8 bgPatternTableNum : 1; /* Number of the Pattern table that the background is stored in. */
		u8 spritePixelSize : 1; /* Size of the sprites in pixels - 8x8 if this is 0, otherwise 8x16 if this is 1. */
		u8 control1Unused : 1; /* Unused bit. */
		u8 vBlankTriggerNMI : 1; /* Whether or not an NMI should be triggered upon a V-Blank. */
	};

	union
	{
		/* PPU Control Register 2 */
		u8 control2;

		u8 colorMode : 1; /* Indicates colour (0) or monochrome mode (1). */
		u8 clipBg : 1; /* Whether or not to clip the background (AKA hide the background in the left 8 pixels on the screen). */
		u8 clipSprites : 1; /* Whether or not to clip the sprites (AKA hide the sprites in the left 8 pixels on the screen). */
		u8 displayBg : 1; /* Does not display the background if 0. */
		u8 displaySprites : 1; /* Does not display sprites if 0. */
		u8 colorModeInfo : 3; /* Indicates background colour in monochrome mode or colour intensity in colour mode. */
	};

	union
	{
		/* Status Register */
		/* (Should be read only) */
		u8 status;

		u8 statusUnused : 4; /* Unused bits. */
		u8 ignoreVramWrites : 1; /* If set to 1, writes to VRAM will be ignored. */
		u8 scanlineSpritesCount : 1; /* If set to 1, indicates that more than 8 sprites are on the current scanline. */
		u8 sprite0Hit : 1; /* Set when a non-transparent pixel of Sprite 0 overlaps a non-transparent background pixel. */
		u8 vBlankInProgress : 1; /* Whether or not a V-Blank is in progress. */
	};

	u8 sprRamAddr; /* The address in SPR-RAM to access on the next write to sprRamIO. */
	u8 sprRamIO; /* Writes a byte to SPR-RAM at the address indicated by sprRamAddr. */

	u16 vramAddr; /* VRAM address register. */
	u8 vramIO; /* Writes a byte to VRAM at the address indicated by vramAddr. */
};

/**
* Handles emulation of the 2C02 PPU used in the NES.
*/
class NESPPU
{
public:
	NESPPU(NESPPUMemoryBus& memBus);
	~NESPPU();

private:
	NESPPUMemoryBus& memBus_;
};

