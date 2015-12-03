#include "NESEmulator.h"

#include <fstream>


NESEmulator::NESEmulator(sf::RenderTarget& target) :
target_(target)//,
//ppuMap_(ppuMem_),
//ppu_(ppuMap_),
//cpuMap_(cpuRam_, ppu_.GetPPURegisters()),
//cpu_(cpuMap_)
{
}


NESEmulator::~NESEmulator()
{
}


void NESEmulator::LoadROM(const std::string& fileName)
{
	// @TODO
	//cart_.LoadROM(fileName);
}


void NESEmulator::RunFrame()
{
	// @TODO
}