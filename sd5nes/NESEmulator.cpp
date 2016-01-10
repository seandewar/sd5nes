#include "NESEmulator.h"

#include <fstream>
#include <iostream> // @TODO DEBUG!

#include <SFML\Graphics\Sprite.hpp>
#include <SFML\Graphics\Texture.hpp>


NESEmulator::NESEmulator(sf::RenderTarget& target) :
target_(target),
ppu_(debug_) // @TODO DEBUG!
{
	// @TODO DEBUG!!
	debug_.create(341, 262, sf::Color::Black);
}


NESEmulator::~NESEmulator()
{
}


void NESEmulator::LoadROM(const std::string& fileName)
{
	// @TODO debugdebugdebug
	cart_.LoadROM(fileName);

	auto mmc = cart_.GetMMC();
	assert(mmc != nullptr);
	cpuComm_ = std::make_unique<NESCPUEmuComm>(cpuRam_, ppu_, *mmc);
	ppuComm_ = std::make_unique<NESPPUEmuComm>(ppuMem_, cpu_, *mmc, cart_.GetMirroringType());

	cpu_.Initialize(*cpuComm_);
	ppu_.Initialize(*ppuComm_);

	cpu_.Power();
	ppu_.Power();

	for (unsigned int i = 0; i < 3; ++i)
		ppu_.Tick();
}


void NESEmulator::Frame()
{
	sf::Sprite spr;
	sf::Texture tex;

	// Keep ticking until a frame is fully rendered by the PPU.
	do
	{
		cpu_.Tick();

		for (unsigned int i = 0; i < 3; ++i)
		{
			ppu_.Tick();
			if (ppu_.IsFrameFinished())
				break;
		}
	} while (!ppu_.IsFrameFinished());

	tex.loadFromImage(debug_);
	spr.setTexture(tex);
	spr.setPosition(0.0f, 0.0f);
	spr.setScale(2.0f, 2.0f);
	target_.draw(spr);
}