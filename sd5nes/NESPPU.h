#pragma once

#include "NESPPUMemoryBus.h"

/**
* Struct containing the registers used by the NES PPU.
*/
struct NESPPURegisters
{
	union
	{
		u8 control1;

		u8 nameTableNum : 2;
		u8 incAddr : 1;
		u8 spritePatternTableNum : 1;
		u8 bgPatternTableNum : 1;
		u8 spritePixelSize : 1;
		u8 control1Unused : 1;
		u8 vBlankTriggerNMI : 1;
	};

	union
	{
		u8 control2;

		u8 colorMode : 1;
		u8 clipBg : 1;
		u8 clipSprites : 1;
		u8 displayBg : 1;
		u8 displaySprites : 1;
		u8 colorModeInfo : 3;
	};

	union
	{
		// Status register is read-only.
		u8 status;

		u8 statusUnused : 4;
		u8 ignoreWrites : 1;
		u8 scanlineSpritesCount : 1;
		u8 sprite0Hit : 1;
		u8 vBlankInProgress : 1;
	};
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

