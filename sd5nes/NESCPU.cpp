#include "NESCPU.h"

#include <cassert>


std::map<u8, NESCPUOpInfo> NESCPU::opInfos_;
NESCPUStaticInit NESCPU::staticInit_;


void NESCPU::RegisterOpMapping(u8 op, NESOpFuncPointer opFunc, NESCPUOpAddressingMode addrMode, int cycleCount)
{
	assert(opInfos_.emplace(op, NESCPUOpInfo(opFunc, addrMode, cycleCount)).second);
}


NESCPUStaticInit::NESCPUStaticInit()
{
	NESCPU::RegisterOpMapping(NES_OP_ADC_IMMEDIATE, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::IMMEDIATE, 2);
}


NESCPU::NESCPU(NESMemory& memory) :
mem_(memory)
{
}


NESCPU::~NESCPU()
{
}


NESCPUOpAddressingMode NESCPU::GetOpAddressingMode(u8 op)
{
	const auto it = opInfos_.find(op);
	if (it == opInfos_.end())
		return NESCPUOpAddressingMode::UNKNOWN;

	return it->second.addrMode;
}


bool NESCPU::ExecuteNextOp()
{
	// Get the next opcode.
	u8 op;
	if (!mem_.Read8(reg_.PC, &op))
		return false; // Couldn't read next op from memory.

	// Locate the mapping for this opcode.
	const auto it = opInfos_.find(op);
	if (it == opInfos_.end())
		return false; // Unknown opcode.

	// Execute the opcode func.
	return (this->*it->second.opFunc)();
}


bool NESCPU::ExecuteOpADC()
{
	return false;
}