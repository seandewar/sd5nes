#include "NESEmulator.h"


NESEmulator::NESEmulator(sf::RenderTarget& target) :
target_(target),
memBus_(ram_, prgRom_),
cpu_(memBus_)
{
}


NESEmulator::~NESEmulator()
{
}
