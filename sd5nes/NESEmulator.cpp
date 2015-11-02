#include "NESEmulator.h"

#include <fstream>


NESEmulator::NESEmulator(sf::RenderTarget& target) :
target_(target),
memBus_(ram_, cart_.GetProgramROM()),
cpu_(memBus_)
{
}


NESEmulator::~NESEmulator()
{
}


bool NESEmulator::LoadROM(const std::string& fileName)
{
	// @TODO Maybe?
	return cart_.LoadROM(fileName);
}


bool NESEmulator::RunFrame()
{

}