#pragma once

#include <SFML\Graphics\RenderTarget.hpp>

#include "NESCPU.h"
#include "NESGamePak.h"

/**
* Class that handles emulation for an instance of the NES system.
*/
class NESEmulator
{
public:
	NESEmulator(sf::RenderTarget& target);
	~NESEmulator();

	// Loads a ROM into PRGROM memory.
	bool LoadROM(const std::string& fileName);

	// Runs a frame of emulation.
	bool RunFrame();

private:
	sf::RenderTarget& target_;

	NESCPU cpu_;

	NESGamePak cart_;
	NESMemory ram_;
	NESMemoryBus memBus_;
};

