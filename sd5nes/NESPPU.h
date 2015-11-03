#pragma once

#include "NESPPUMemoryBus.h"

/**
* Struct containing the registers used by the NES PPU.
*/
struct NESPPURegisters
{
	union
	{
		u8 Control;

		u8 xScrollNt : 1;
		u8 yScrollNt : 1;
		u8 incAddr : 1;
		// @TODO
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

