#pragma once

#include <SFML\Graphics\RenderTarget.hpp>

#include "NESCPU.h"
#include "NESPPU.h"
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
	void LoadROM(const std::string& fileName);

	// Runs one frame of emulation.
	void RunFrame();

private:
	sf::RenderTarget& target_;

	NESGamePak cart_;

	NESPPUMemory ppuMem_;
	NESPPUMemoryMap ppuMap_;
	NESPPU ppu_;

	NESMemCPURAM cpuRam_;
	NESCPUMemoryMap cpuMap_;
	NESCPU cpu_;
};

