#include "NESEmulator.h"

#include <fstream>
#include <iostream> // @TODO DEBUG!

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Graphics/Text.hpp> //@TODO DEBUG!


NESEmulator::NESEmulator(sf::RenderTarget& target, const sf::Font& debugFont) :
target_(target),
ppu_(debug_), // @TODO DEBUG!
debugFont_(debugFont)
{
	// Init controller ports
	for (auto& port : controllers_)
		port = nullptr;
}


NESEmulator::~NESEmulator()
{
}


bool NESEmulator::AddController(NESControllerPort port, INESController& controller)
{
	std::size_t controllerIndex;
	switch (port)
	{
	case NESControllerPort::CONTROLLER_1:
		controllerIndex = 0;
		break;

	case NESControllerPort::CONTROLLER_2:
		controllerIndex = 1;
		break;

	default:
		assert(false && "Unknown controller port!");
		return false;
	}

	if (controllers_[controllerIndex] != nullptr)
		return false;

	controllers_[controllerIndex] = &controller;
	return true;
}


bool NESEmulator::RemoveController(NESControllerPort port)
{
	std::size_t controllerIndex;
	switch (port)
	{
	case NESControllerPort::CONTROLLER_1:
		controllerIndex = 0;
		break;

	case NESControllerPort::CONTROLLER_2:
		controllerIndex = 1;
		break;

	default:
		assert(false && "Unknown controller port!");
		return false;
	}

	if (controllers_[controllerIndex] == nullptr)
		return false;

	controllers_[controllerIndex] = nullptr;
	return true;
}


void NESEmulator::LoadROM(const std::string& fileName)
{
	// @TODO: debugdebugdebug
	cart_.LoadROM(fileName);
	cartState_ = cart_.GetNewGamePakPowerState();

	cpuComm_ = std::make_unique<NESCPUEmuComm>(cpuRam_, ppu_, cartState_->GetMMC(), controllers_);
	ppuComm_ = std::make_unique<NESPPUEmuComm>(ppuMem_, cpu_, cartState_->GetMMC(), cartState_->GetNameTableMirroringRef());

	cpu_.Initialize(*cpuComm_);
	ppu_.Initialize(*ppuComm_);

	cpu_.Power();
	ppu_.Power();
}


void NESEmulator::Frame()
{
	// @TODO So much debug!!1
	sf::Sprite spr;
	sf::Texture tex;

	// @TODO: DEBUG!!
	debug_.create(341, 262, ppu_.GetBackdropColor().ToSFColor());

	// Keep ticking until a frame is fully rendered by the PPU.
	const auto elapsedFrames = ppu_.GetElapsedFramesCount();

    // @TODO: Debug information to console
    std::cout << "F " << elapsedFrames << ", C " << cpu_.GetElapsedCycles() << std::endl;
    std::cout << " CPU: " << cpu_.GetRegisters().ToString() << std::endl;
    std::cout << " PPU: " << ppu_.GetRegisters().ToString() << std::endl;

	while (elapsedFrames == ppu_.GetElapsedFramesCount())
	{
		cpu_.Tick();

		ppu_.Tick();
		ppu_.Tick();
		ppu_.Tick();
	}

	tex.loadFromImage(debug_);
	spr.setTexture(tex);
	target_.draw(spr);
}
