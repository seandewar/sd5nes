#pragma once

#include <SFML\Graphics\RenderTarget.hpp>

#include "NESCPU.h"

/**
* Class that handles emulation for an instance of the NES system.
*/
class NESEmulator
{
public:
	NESEmulator(sf::RenderTarget& target);
	~NESEmulator();

	// Runs a frame of emulation.
	bool RunFrame();

private:
	sf::RenderTarget& target_;

	NESCPU cpu_;

	NESMemoryRAM ram_;
	NESMemoryPRGROM prgRom_;
	NESMemoryBus memBus_;
};

