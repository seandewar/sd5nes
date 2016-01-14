#pragma once

#include <memory>

#include <SFML\Graphics\RenderTarget.hpp>

#include "NESCPU.h"
#include "NESCPUEmuComm.h"
#include "NESPPU.h"
#include "NESPPUEmuComm.h"
#include "NESGamePak.h"
#include "NESController.h"

/**
* Enum containing the different numbers of the controller ports on the NES.
*/
enum class NESControllerPort
{
	CONTROLLER_1,
	CONTROLLER_2
};

/**
* Class that handles emulation for an instance of the NES system.
*/
class NESEmulator
{
public:
	NESEmulator(sf::RenderTarget& target);
	~NESEmulator();

	/**
	* Adds a controller to the specified controller port if it is empty.
	* Returns true if the port was empty and the controller was added, false otherwise.
	*/
	bool AddController(NESControllerPort port, INESController& controller);

	/**
	* Removes the controller from the specified controller port.
	* Returns true if a controller was removed from the port, false if the port was already empty.
	*/
	bool RemoveController(NESControllerPort port);

	/**
	* Loads a ROM.
	*/
	void LoadROM(const std::string& fileName);

	/**
	* Runs one frame of emulation.
	*/
	void Frame();

private:
	sf::RenderTarget& target_;
	
	// @TODO Debug!! everyWHERE!!
	sf::Image debug_;

	NESControllerPorts controllers_;

	NESGamePak cart_;

	NESMemCPURAM cpuRam_;
	std::unique_ptr<NESCPUEmuComm> cpuComm_;
	NESCPU cpu_;

	NESPPUMemory ppuMem_;
	std::unique_ptr<NESPPUEmuComm> ppuComm_;
	NESPPU ppu_;
};
