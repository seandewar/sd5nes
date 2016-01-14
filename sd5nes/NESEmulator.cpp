#include "NESEmulator.h"

#include <fstream>
#include <iostream> // @TODO DEBUG!

#include <SFML\Graphics\Sprite.hpp>
#include <SFML\Graphics\Texture.hpp>


NESEmulator::NESEmulator(sf::RenderTarget& target) :
target_(target),
ppu_(debug_) // @TODO DEBUG!
{
	// Init controller ports
	for (auto& port : controllers_)
		port = nullptr;

	// @TODO: DEBUG!!
	debug_.create(341, 262, sf::Color::Black);
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

	auto mmc = cart_.GetMMC();
	assert(mmc != nullptr);

	cpuComm_ = std::make_unique<NESCPUEmuComm>(cpuRam_, ppu_, *mmc, controllers_);
	ppuComm_ = std::make_unique<NESPPUEmuComm>(ppuMem_, cpu_, *mmc, cart_.GetMirroringType());

	cpu_.Initialize(*cpuComm_);
	ppu_.Initialize(*ppuComm_);

	cpu_.Power();
	ppu_.Power();
}


void NESEmulator::Frame()
{
	sf::Sprite spr;
	sf::Texture tex;

	// Keep ticking until a frame is fully rendered by the PPU.
	const auto elapsedFrames = ppu_.GetElapsedFramesCount();
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