#include "NESCPU.h"

#include "NESCPUConstants.h"

#include <cassert>


std::map<u8, NESCPUOpInfo> NESCPU::opInfos_;
NESCPUStaticInit NESCPU::staticInit_;


void NESCPU::RegisterOpMapping(u8 op, NESOpFuncPointer opFunc, NESCPUOpAddressingMode addrMode, int cycleCount)
{
	assert((opInfos_.emplace(op, NESCPUOpInfo(opFunc, addrMode, cycleCount)).second)
		&& (addrMode != NESCPUOpAddressingMode::UNKNOWN));
}


NESCPUStaticInit::NESCPUStaticInit()
{
	// ADC
	NESCPU::RegisterOpMapping(NES_OP_ADC_IMMEDIATE, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_ADC_ABSOLUTE, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_ADC_ABSOLUTE_X, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::ABSOLUTE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_ADC_ABSOLUTE_Y, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::ABSOLUTE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_ADC_ZEROPAGE, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_ADC_ZEROPAGE_X, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_ADC_INDIRECT_X, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::INDIRECT_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_ADC_INDIRECT_Y, &NESCPU::ExecuteOpADC, NESCPUOpAddressingMode::INDIRECT_Y, 5);

	// AND
	NESCPU::RegisterOpMapping(NES_OP_AND_IMMEDIATE, &NESCPU::ExecuteOpAND, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_AND_ABSOLUTE, &NESCPU::ExecuteOpAND, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_AND_ABSOLUTE_X, &NESCPU::ExecuteOpAND, NESCPUOpAddressingMode::ABSOLUTE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_AND_ABSOLUTE_Y, &NESCPU::ExecuteOpAND, NESCPUOpAddressingMode::ABSOLUTE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_AND_ZEROPAGE, &NESCPU::ExecuteOpAND, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_AND_ZEROPAGE_X, &NESCPU::ExecuteOpAND, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_AND_INDIRECT_X, &NESCPU::ExecuteOpAND, NESCPUOpAddressingMode::INDIRECT_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_AND_INDIRECT_Y, &NESCPU::ExecuteOpAND, NESCPUOpAddressingMode::INDIRECT_Y, 5);

	// ASL
	NESCPU::RegisterOpMapping(NES_OP_ASL_ACCUMULATOR, &NESCPU::ExecuteOpASL, NESCPUOpAddressingMode::ACCUMULATOR, 2);
	NESCPU::RegisterOpMapping(NES_OP_ASL_ZEROPAGE, &NESCPU::ExecuteOpASL, NESCPUOpAddressingMode::ZEROPAGE, 5);
	NESCPU::RegisterOpMapping(NES_OP_ASL_ZEROPAGE_X, &NESCPU::ExecuteOpASL, NESCPUOpAddressingMode::ZEROPAGE_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_ASL_ABSOLUTE, &NESCPU::ExecuteOpASL, NESCPUOpAddressingMode::ABSOLUTE, 6);
	NESCPU::RegisterOpMapping(NES_OP_ASL_ABSOLUTE_X, &NESCPU::ExecuteOpASL, NESCPUOpAddressingMode::ABSOLUTE_X, 7);
}


NESCPU::NESCPU(NESMemory& memory) :
mem_(memory),
currentOp_(NES_OP_INVALID),
currentOpCycleCount_(0)
{
}


NESCPU::~NESCPU()
{
}


u8 NESCPU::GetCurrentOpcode() const
{
	return currentOp_;
}


NESCPUOpAddressingMode NESCPU::GetOpAddressingMode(u8 op)
{
	const auto it = opInfos_.find(op);
	if (it == opInfos_.end())
		return NESCPUOpAddressingMode::UNKNOWN;

	return it->second.addrMode;
}


bool NESCPU::GetOpSizeFromAddressingMode(NESCPUOpAddressingMode addrMode, int* outOpSize)
{
	int opSize;
	switch (addrMode)
	{
	// 1 byte addressing modes.
	case NESCPUOpAddressingMode::ACCUMULATOR:
		opSize = 1;
		break;

	// 2 byte addressing modes.
	case NESCPUOpAddressingMode::IMMEDIATE:
	case NESCPUOpAddressingMode::ZEROPAGE:
	case NESCPUOpAddressingMode::ZEROPAGE_X:
	case NESCPUOpAddressingMode::INDIRECT_X:
	case NESCPUOpAddressingMode::INDIRECT_Y:
		opSize = 2;
		break;

	// 3 byte addressing modes.
	case NESCPUOpAddressingMode::ABSOLUTE:
	case NESCPUOpAddressingMode::ABSOLUTE_X:
	case NESCPUOpAddressingMode::ABSOLUTE_Y:
		opSize = 3;
		break;

	// Unknown addressing mode.
	default:
		return false;
	}

	// Write to outOpSize if not null.
	if (outOpSize != nullptr)
		*outOpSize = opSize;

	return true;
}


bool NESCPU::ExecuteNextOp()
{
	// Get the next opcode.
	if (!mem_.Read8(reg_.PC, &currentOp_))
		return false;

	// Locate the mapping for this opcode.
	const auto it = opInfos_.find(currentOp_);
	if (it == opInfos_.end())
		return false; // Unknown opcode.

	// Execute the opcode func.
	if (!(this->*it->second.opFunc)())
		return false; // Failed to execute opcode - program execution error.

	int opSize;
	if (!GetOpSizeFromAddressingMode(it->second.addrMode, &opSize))
		return false;

	// Go to the next instruction.
	reg_.PC += opSize;
	return true;
}


bool NESCPU::ReadOpArgValue(u8* outVal, bool* outCrossedPageBoundary)
{
	const auto it = opInfos_.find(currentOp_);
	if (it == opInfos_.end())
		return false;

	// TODO : CHECK IF CROSSED PAGE BOUNDARY!!!!!11111111111
	u16 addr = 0x0000;
	bool crossedPageBoundary = false; // Assume false.
	switch (it->second.addrMode)
	{

		// Immediate - read the next byte as a value.
	case NESCPUOpAddressingMode::IMMEDIATE:
		addr = reg_.PC + 1;
		break;


		// Absolute - read the next 2 bytes as an address.
	case NESCPUOpAddressingMode::ABSOLUTE:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;
		break;


		// Absolute, X - read the next 2 bytes as an address and offset by X.
	case NESCPUOpAddressingMode::ABSOLUTE_X:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;

		addr += reg_.X;
		break;


		// Absolute, Y - read the next 2 bytes as an address and offset by Y.
	case NESCPUOpAddressingMode::ABSOLUTE_Y:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;

		addr += reg_.Y;
		break;


		// 8-bit representation of address used to read Zero Page and Indirect addressing types.
		u8 addr8;


		// ZeroPage - read the next byte as an address (convert to 2 bytes where most-significant byte is 0).
	case NESCPUOpAddressingMode::ZEROPAGE:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		addr = addr8;
		break;


		// ZeroPage, X - read the next byte as an address (convert to 2 bytes where most-significant byte is 0) + X.
	case NESCPUOpAddressingMode::ZEROPAGE_X:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		addr8 += reg_.X; // Will wrap around if X is too big.
		addr = addr8;
		break;


		// (Indirect, X) - read the next byte as an address (convert to 2 bytes where most-significant byte is 0) + X.
	case NESCPUOpAddressingMode::INDIRECT_X:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		addr8 += reg_.X; // Will wrap around if X is too big.

		// Read address from memory at addr8 into addr.
		if (!mem_.Read16(addr8, &addr))
			return false;
		break;


		// (Indirect), Y - read the next byte as an address (convert to 2 bytes where most-significant byte is 0). Add 
	case NESCPUOpAddressingMode::INDIRECT_Y:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		// Read address from memory at addr8 into addr.
		if (!mem_.Read16(addr8, &addr))
			return false;

		addr += reg_.Y;
		break;


		// Accumulator - read from the accumulator.
	case NESCPUOpAddressingMode::ACCUMULATOR:
		if (outVal != nullptr)
			*outVal = reg_.A;

		if (outCrossedPageBoundary != nullptr)
			*outCrossedPageBoundary = crossedPageBoundary;
		return true;


		// Unknown address mode...
	default:
		return false;

	}

	// Assume reading from memory.
	// Write to outVal if not null.
	if (outVal != nullptr)
	{
		if (!mem_.Read8(addr, outVal))
			return false; // Failed to read from address - program error.
	}

	// Write to crossedPageBoundary if not null.
	if (outCrossedPageBoundary != nullptr)
		*outCrossedPageBoundary = crossedPageBoundary;

	return true;
}


bool NESCPU::WriteOpResult(u8 result)
{
}


bool NESCPU::ExecuteOpADC()
{
	u8 val;
	bool crossedPageBoundary;
	if (!ReadOpArgValue(&val, &crossedPageBoundary))
		return false;

	// ADC takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A + M + C -> A, C
	// NOTE: NES 6502 variant has no BCD mode.
	const uleast16 res = reg_.A + val + reg_.C;

	reg_.C = (res > 0xFF ? 1 : 0);
	reg_.N = ((res >> 7) & 1); // Check if sign bit is 1.
	reg_.V = (((~(reg_.A ^ val) & (reg_.A ^ res)) >> 7) & 1); // Check if the sign has changed due to overflow.
	reg_.A = static_cast<u8>(res);
	return true;
}


bool NESCPU::ExecuteOpAND()
{
	u8 val;
	bool crossedPageBoundary;
	if (!ReadOpArgValue(&val, &crossedPageBoundary))
		return false;

	// AND takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A AND M -> A
	const u8 res = (reg_.A & val);

	reg_.Z = (res == 0 ? 1 : 0);
	reg_.N = ((res >> 7) & 1); // Check if sign bit is 1.
	reg_.A = res;
	return true;
}


bool NESCPU::ExecuteOpASL()
{
	// TODO
}