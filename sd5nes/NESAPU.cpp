#include "NESAPU.h"


NESAPU::NESAPU() :
regStatus_(0)
{
}


NESAPU::~NESAPU()
{
}


u8 NESAPU::ReadRegister(NESAPURegisterType type)
{
	switch (type)
	{
	case NESAPURegisterType::STATUS_CONTROL:
		return regStatus_;

	default:
		// Every other register is write-only.
		return 0;
	}
}


void NESAPU::WriteRegister(NESAPURegisterType type, u8 val)
{
	// @TODO
}


void NESAPU::Tick()
{

}