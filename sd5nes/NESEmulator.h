#pragma once

#include <memory>

#include <SFML\Graphics\RenderTarget.hpp>

#include "NESCPU.h"
#include "NESCPUEmuComm.h"
#include "NESPPU.h"
#include "NESPPUEmuComm.h"
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

	NESMemCPURAM cpuRam_;
	std::unique_ptr<NESCPUEmuComm> cpuComm_;
	NESCPU cpu_;

	NESPPUMemory ppuMem_;
	std::unique_ptr<NESPPUEmuComm> ppuComm_;
	NESPPU ppu_;
};
