#include "NESEmulator.h"

#include <fstream>

#include <SFML\Graphics\Sprite.hpp>
#include <SFML\Graphics\Texture.hpp>


NESEmulator::NESEmulator(sf::RenderTarget& target) :
target_(target),
ppu_(debug_) // @TODO DEBUG!
{
}


NESEmulator::~NESEmulator()
{
}


void NESEmulator::LoadROM(const std::string& fileName)
{
	// @TODO debugdebugdebug
	cart_.LoadROM(fileName);

	cpuComm_ = std::make_unique<NESCPUEmuComm>(cpuRam_, ppu_, cart_.GetMMC());
	ppuComm_ = std::make_unique<NESPPUEmuComm>(ppuMem_, cpu_, cart_.GetMirroringType());

	cpu_.Initialize(*cpuComm_);
	ppu_.Initialize(*ppuComm_);

	cpu_.Power();
}


void NESEmulator::Frame()
{
	sf::Sprite spr;
	sf::Texture tex;

	// @TODO DEBUG!!
	debug_.create(341, 262, sf::Color::Black);

	for (int i = 0; i < 30000; ++i)
	{
		const auto clocks = cpu_.Tick(); // @TODO DEBUG!
		for (int j = 0; j < clocks; ++j)
		{
			ppu_.Tick();
			ppu_.Tick();
			ppu_.Tick();
		}
	}

	tex.loadFromImage(debug_);
	spr.setTexture(tex);
	spr.setScale(2.0f, 2.0f);
	target_.draw(spr);
}