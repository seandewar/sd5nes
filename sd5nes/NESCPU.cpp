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

	// BCC
	NESCPU::RegisterOpMapping(NES_OP_BCC_RELATIVE, &NESCPU::ExecuteOpBCC, NESCPUOpAddressingMode::RELATIVE, 2);

	// BCS
	NESCPU::RegisterOpMapping(NES_OP_BCS_RELATIVE, &NESCPU::ExecuteOpBCS, NESCPUOpAddressingMode::RELATIVE, 2);

	// BEQ
	NESCPU::RegisterOpMapping(NES_OP_BEQ_RELATIVE, &NESCPU::ExecuteOpBEQ, NESCPUOpAddressingMode::RELATIVE, 2);

	// BIT
	NESCPU::RegisterOpMapping(NES_OP_BIT_ZEROPAGE, &NESCPU::ExecuteOpBIT, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_BIT_ABSOLUTE, &NESCPU::ExecuteOpBIT, NESCPUOpAddressingMode::ABSOLUTE, 4);
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
	case NESCPUOpAddressingMode::RELATIVE:
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
	currentOpMappingIt_ = it;
	currentOpCycleCount_ = it->second.cycleCount;
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
	// TODO : CHECK IF CROSSED PAGE BOUNDARY!!!!!11111111111
	u16 addr;
	bool crossedPageBoundary = false; // Assume false.
	switch (currentOpMappingIt_->second.addrMode)
	{
	case NESCPUOpAddressingMode::ACCUMULATOR:
		// Read from accumulator instead.
		if (outVal != nullptr)
			*outVal = reg_.A;

		if (outCrossedPageBoundary != nullptr)
			*outCrossedPageBoundary = false;
		return true;


	case NESCPUOpAddressingMode::IMMEDIATE:
	case NESCPUOpAddressingMode::RELATIVE:
		addr = reg_.PC + 1;
		break;


	case NESCPUOpAddressingMode::ABSOLUTE:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;
		break;


	case NESCPUOpAddressingMode::ABSOLUTE_X:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;

		addr += reg_.X;
		break;


	case NESCPUOpAddressingMode::ABSOLUTE_Y:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;

		addr += reg_.Y;
		break;


		// 8-bit representation of address used to read Zero Page and Indirect addressing types.
		u8 addr8;


	case NESCPUOpAddressingMode::ZEROPAGE:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		addr = addr8;
		break;


	case NESCPUOpAddressingMode::ZEROPAGE_X:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		addr8 += reg_.X; // Will wrap around if X is too big.
		addr = addr8;
		break;


	case NESCPUOpAddressingMode::INDIRECT_X:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		addr8 += reg_.X; // Will wrap around if X is too big.

		// Read address from memory at addr8 into addr.
		if (!mem_.Read16(addr8, &addr))
			return false;
		break;


	case NESCPUOpAddressingMode::INDIRECT_Y:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		// Read address from memory at addr8 into addr.
		if (!mem_.Read16(addr8, &addr))
			return false;

		addr += reg_.Y;
		break;


		// Unhandled addressing mode!
	default:
		return false;
	}

	// Assume reading from main memory.
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
	u16 addr;
	switch (currentOpMappingIt_->second.addrMode)
	{
	case NESCPUOpAddressingMode::ACCUMULATOR:
		// Write to accumulator instead.
		reg_.A = result;
		return true;


	case NESCPUOpAddressingMode::ABSOLUTE:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;


	case NESCPUOpAddressingMode::ABSOLUTE_X:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;

		addr += reg_.X;
		break;


		// 8-bit representation of address used to read Zero Page and Indirect addressing types.
		u8 addr8;


	case NESCPUOpAddressingMode::ZEROPAGE:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		addr = addr8;
		break;


	case NESCPUOpAddressingMode::ZEROPAGE_X:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		addr8 += reg_.X; // Will wrap around if X is too big.
		addr = addr8;
		break;


		// Unhandled addressing mode!
	default:
		return false;
	}

	// Assume writing to main memory at addr.
	return mem_.Write8(addr, result);
}


bool NESCPU::ExecuteOpADC()
{
	u8 argVal;
	bool crossedPageBoundary;
	if (!ReadOpArgValue(&argVal, &crossedPageBoundary))
		return false;

	// ADC takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A + M + C -> A, C
	// NOTE: NES 6502 variant has no BCD mode.
	const uleast16 res = reg_.A + argVal + reg_.C;

	reg_.C = (res > 0xFF ? 1 : 0);
	UpdateRegN(res);
	reg_.V = (((~(reg_.A ^ argVal) & (reg_.A ^ res)) >> 7) & 1); // Check if the sign has changed due to overflow.
	reg_.A = static_cast<u8>(res);
	return true;
}


bool NESCPU::ExecuteOpAND()
{
	u8 argVal;
	bool crossedPageBoundary;
	if (!ReadOpArgValue(&argVal, &crossedPageBoundary))
		return false;

	// AND takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A AND M -> A
	const u8 res = (reg_.A & argVal);

	UpdateRegZ(res);
	UpdateRegN(res);
	reg_.A = res;
	return true;
}


bool NESCPU::ExecuteOpASL()
{
	u8 argVal;
	if (!ReadOpArgValue(&argVal, nullptr))
		return false;

	// C <- [76543210] <- 0
	const u8 res = (argVal << 1);
	WriteOpResult(res);

	reg_.C = ((argVal >> 7) & 1); // Set carry bit if bit 7 was originally 1.
	UpdateRegZ(res);
	UpdateRegN(res);
	return true;
}


bool NESCPU::ExecuteOpAsBranch(bool shouldBranch)
{
	if (!shouldBranch)
		return true;

	u8 argVal;
	if (!ReadOpArgValue(&argVal, nullptr))
		return false;

	const u16 jumpPC = reg_.PC + argVal;

	// Check if the branch will cross a page boundary.
	if ((reg_.PC & 0xFF00) != (jumpPC & 0xFF00))
		currentOpCycleCount_ += 2;
	else
		currentOpCycleCount_ += 1;

	reg_.PC = jumpPC;
	return true;
}


bool NESCPU::ExecuteOpBCC()
{
	// Branch on C = 0
	return ExecuteOpAsBranch((reg_.C == 0));
}


bool NESCPU::ExecuteOpBCS()
{
	// Branch on C = 1
	return ExecuteOpAsBranch((reg_.C == 1));
}


bool NESCPU::ExecuteOpBEQ()
{
	// Branch on Z = 1
	return ExecuteOpAsBranch((reg_.Z == 1));
}


bool NESCPU::ExecuteOpBIT()
{
	u8 argVal;
	if (!ReadOpArgValue(&argVal, nullptr))
		return false;

	// A /\ M, M7 -> N, M6 -> V
	const u8 res = (argVal << 1);
	WriteOpResult(res);

	reg_.C = ((argVal >> 7) & 1); // Set carry bit if bit 7 was originally 1.
	UpdateRegZ(res);
	UpdateRegN(res);
	return true;
}