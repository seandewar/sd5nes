#include "NESAPU.h"


NESAPU::NESAPU()
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
		// @TODO
		return 0;

	default:
		// Every other register is write-only.
		return 0;
	}
}


void NESAPU::WriteRegister(NESAPURegisterType type, u8 val)
{
	// @TODO
}