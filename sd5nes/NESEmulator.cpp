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
	try
	{
		cart_.LoadROM(fileName);
	}
	catch (const NESGamePakLoadException& ex)
	{
		return false;
	}

	return true;
}


bool NESEmulator::RunFrame()
{
	// @TODO
	return false;
}