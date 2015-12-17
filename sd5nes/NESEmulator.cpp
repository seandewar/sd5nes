#include "NESEmulator.h"

#include <fstream>

#include <SFML\Graphics\Sprite.hpp>
#include <SFML\Graphics\Texture.hpp>


NESEmulator::NESEmulator(sf::RenderTarget& target) :
target_(target)
{
}


NESEmulator::~NESEmulator()
{
}


void NESEmulator::LoadROM(const std::string& fileName)
{
	// @TODO debugdebugdebug
	cart_.LoadROM(fileName);

	ppuMap_ = std::make_unique<NESPPUMemoryMapper>(ppuMem_, cart_.GetMirroringType());
	ppu_ = std::make_unique<NESPPU>(*ppuMap_, debug_);

	cpuMap_ = std::make_unique<NESCPUMemoryMapper>(cpuRam_, *ppu_, cart_.GetMMC());
	cpu_ = std::make_unique<NESCPU>(*cpuMap_);
}


void NESEmulator::Frame()
{
	sf::Sprite spr;
	sf::Texture tex;

	// @TODO DEBUG!!
	debug_.create(256, 240, sf::Color::Black);

	cpu_->Run(20480); // @TODO DEBUG!!
	ppu_->Frame();

	tex.loadFromImage(debug_);
	spr.setTexture(tex);
	spr.setScale(2.4f, 2.4f);
	target_.draw(spr);
}