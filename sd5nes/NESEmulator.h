#pragma once

#include <memory>

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

	// Loads a ROM into PRG-ROM memory.
	void LoadROM(const std::string& fileName);

	// Runs one frame of emulation.
	void Frame();

private:
	sf::RenderTarget& target_;
	
	// @TODO Debug!! everyWHERE!!
	sf::Image debug_;

	NESGamePak cart_;

	NESPPUMemory ppuMem_;
	std::unique_ptr<NESPPUMemoryMapper> ppuMap_;
	std::unique_ptr<NESPPU> ppu_;

	NESMemCPURAM cpuRam_;
	std::unique_ptr<NESCPUMemoryMapper> cpuMap_;
	std::unique_ptr<NESCPU> cpu_;
};
