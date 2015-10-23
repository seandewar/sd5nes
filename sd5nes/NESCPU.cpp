#include "NESCPU.h"

#include "NESCPUOpConstants.h"


NESCPU::NESCPU(NESMemory& memory) :
mem_(memory)
{
}


NESCPU::~NESCPU()
{
}


void NESCPU::SetOpUsedCycles(int cycleAmount)
{
	// TODO
}


NESCPUOpAddressingMode NESCPU::GetOpAddressingMode(u8 op)
{
	// TODO - fill with other opcodes.
	switch (op)
	{
	// Immediate
	case NES_OP_ADC_IMMEDIATE:
		return NESCPUOpAddressingMode::IMMEDIATE;

	// Absolute
	case NES_OP_ADC_ABSOLUTE:
		return NESCPUOpAddressingMode::ABSOLUTE;

	// Absolute, X
	case NES_OP_ADC_ABSOLUTE_X:
		return NESCPUOpAddressingMode::ABSOLUTE_X;

	// Absolute, Y
	case NES_OP_ADC_ABSOLUTE_Y:
		return NESCPUOpAddressingMode::ABSOLUTE_Y;

	// ZeroPage
	case NES_OP_ADC_ZEROPAGE:
		return NESCPUOpAddressingMode::ZEROPAGE;

	// ZeroPage, X
	case NES_OP_ADC_ZEROPAGE_X:
		return NESCPUOpAddressingMode::ZEROPAGE_X;

	// (Indirect, X)
	case NES_OP_ADC_INDIRECT_X:
		return NESCPUOpAddressingMode::INDIRECT_X;

	// (Indirect), Y
	case NES_OP_ADC_INDIRECT_Y:
		return NESCPUOpAddressingMode::INDIRECT_Y;
	}

	// Unknown addressing type...
	return NESCPUOpAddressingMode::UNKNOWN;
}


bool NESCPU::ExecuteNextOp()
{
	// Get the next opcode.
	u8 op;
	if (!mem_.Read8(reg_.PC, &op))
		return false;

	// Get addressing mode and execute the correct opcode.
	const auto addrMode = GetOpAddressingMode(op);
	switch (op)
	{
	// Add with Carry (ADC)
	case NES_OP_ADC_ABSOLUTE:
	case NES_OP_ADC_ABSOLUTE_X:
	case NES_OP_ADC_ABSOLUTE_Y:
	case NES_OP_ADC_IMMEDIATE:
	case NES_OP_ADC_INDIRECT_X:
	case NES_OP_ADC_INDIRECT_Y:
	case NES_OP_ADC_ZEROPAGE:
	case NES_OP_ADC_ZEROPAGE_X:
		return ExecuteOpADC(addrMode);

	// TODO
	}

	return false;
}


bool NESCPU::ExecuteOpADC(NESCPUOpAddressingMode addrMode)
{
}