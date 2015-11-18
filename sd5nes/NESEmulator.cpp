#include "NESEmulator.h"

#include <fstream>


NESEmulator::NESEmulator(sf::RenderTarget& target) :
target_(target),
cpu_(NESMemory(0x800)) // @TODO: DEBUG!!!!
{
}


NESEmulator::~NESEmulator()
{
}


void NESEmulator::LoadROM(const std::string& fileName)
{
	// @TODO
	cart_.LoadROM(fileName);
}


void NESEmulator::RunFrame()
{
	// @TODO
}