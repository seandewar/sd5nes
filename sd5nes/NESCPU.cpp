#include "NESCPU.h"

#include <cassert>
#include <sstream>
#include <iostream> // @TODO Debug?


std::string NESCPU::OpAsAsm(const std::string& opName, NESCPUOpAddrMode addrMode, u16 val)
{
	std::ostringstream oss;
	oss << opName << " ";

	switch (addrMode)
	{
	case NESCPUOpAddrMode::IMPLIED:
	case NESCPUOpAddrMode::IMPLIED_BRK:
		break; // No operand in implied instructions.

	case NESCPUOpAddrMode::ACCUMULATOR:
		oss << "A";
		break;

	case NESCPUOpAddrMode::IMMEDIATE:
		oss << "#$" << std::hex << val;
		break;

	case NESCPUOpAddrMode::ZEROPAGE:
		oss << "$" << std::hex << val;
		break;

	case NESCPUOpAddrMode::ZEROPAGE_X:
		oss << "$" << std::hex << val << ",X";
		break;

	case NESCPUOpAddrMode::ZEROPAGE_Y:
		oss << "$" << std::hex << val << ",Y";
		break;

	case NESCPUOpAddrMode::RELATIVE:
	case NESCPUOpAddrMode::ABSOLUTE:
		oss << "$" << std::hex << val;
		break;

	case NESCPUOpAddrMode::ABSOLUTE_X:
		oss << "$" << std::hex << val << ",X";
		break;

	case NESCPUOpAddrMode::ABSOLUTE_Y:
		oss << "$" << std::hex << val << ",Y";
		break;

	case NESCPUOpAddrMode::INDIRECT:
		oss << "($" << std::hex << val << ")";
		break;

	case NESCPUOpAddrMode::INDIRECT_X:
		oss << "($" << std::hex << val << ",X)";
		break;

	case NESCPUOpAddrMode::INDIRECT_Y:
		oss << "($" << std::hex << val << "),Y";
		break;

	default:
		oss << "[???]";
		break;
	}

	return oss.str();
}


NESCPUOpAddrMode NESCPU::GetOpAddrMode(u8 op)
{
	return opInfos_[op].addrMode;
}


u16 NESCPU::GetOpSizeFromAddrMode(NESCPUOpAddrMode addrMode)
{
	switch (addrMode)
	{
		// 1 byte addressing modes.
	case NESCPUOpAddrMode::ACCUMULATOR:
	case NESCPUOpAddrMode::IMPLIED:
		return 1;

		// 2 byte addressing modes.
	case NESCPUOpAddrMode::IMPLIED_BRK: // BRK has a padding byte.
	case NESCPUOpAddrMode::IMMEDIATE:
	case NESCPUOpAddrMode::RELATIVE:
	case NESCPUOpAddrMode::ZEROPAGE:
	case NESCPUOpAddrMode::ZEROPAGE_X:
	case NESCPUOpAddrMode::ZEROPAGE_Y:
	case NESCPUOpAddrMode::INDIRECT_X:
	case NESCPUOpAddrMode::INDIRECT_Y:
		return 2;

		// 3 byte addressing modes.
	case NESCPUOpAddrMode::ABSOLUTE:
	case NESCPUOpAddrMode::ABSOLUTE_X:
	case NESCPUOpAddrMode::ABSOLUTE_Y:
	case NESCPUOpAddrMode::INDIRECT:
		return 3;

		// Unknown addressing mode.
	default:
		assert("Unknown addressing mode supplied!" && false);
		return 0;
	}
}


NESCPU::NESCPU() :
comm_(nullptr),
elapsedCycles_(0)
{
}


NESCPU::~NESCPU()
{
}


void NESCPU::Initialize(INESCPUCommunicationsInterface& comm)
{
	comm_ = &comm;
}


void NESCPU::SetInterrupt(NESCPUInterruptType interrupt)
{
	switch (interrupt)
	{
	case NESCPUInterruptType::RESET:
		intReset_ = true;
		break;

	case NESCPUInterruptType::NMI:
		intNmi_ = true;
		break;

	case NESCPUInterruptType::IRQ:
		intIrq_ = true;
		break;
	}
}


void NESCPU::Power()
{
	assert(comm_ != nullptr);

	elapsedCycles_ = 0;

	stallTicksLeft_ = 0;
	isJammed_ = false;

	intReset_ = true; // Trigger a Reset interrupt.
	intNmi_ = intIrq_ = false;

	reg_.PC = 0xC000;
	reg_.SP = 0xFD;
	reg_.SetP(0x34); // I, B (and bit 5) are set on power.
	reg_.A = reg_.X = reg_.Y = 0;

	// @TODO Memory to power-up state!
}


void NESCPU::WriteOpResult(NESCPUOpAddrMode addrMode, u8 result)
{
	u16 addr;
	switch (addrMode)
	{
	case NESCPUOpAddrMode::ACCUMULATOR:
		reg_.A = result; // Write to accumulator instead.
		return;

	case NESCPUOpAddrMode::INDIRECT_X:
		addr = NESHelper::MemoryIndirectRead16(*comm_, (comm_->Read8(reg_.PC + 1) + reg_.X) & 0xFF);
		break;

	case NESCPUOpAddrMode::INDIRECT_Y:
		addr = NESHelper::MemoryIndirectRead16(*comm_, comm_->Read8(reg_.PC + 1)) + reg_.Y;
		break;

	case NESCPUOpAddrMode::ABSOLUTE:
		addr = NESHelper::MemoryRead16(*comm_, reg_.PC + 1);
		break;

	case NESCPUOpAddrMode::ABSOLUTE_X:
		addr = NESHelper::MemoryRead16(*comm_, reg_.PC + 1) + reg_.X;
		break;

	case NESCPUOpAddrMode::ABSOLUTE_Y:
		addr = NESHelper::MemoryRead16(*comm_, reg_.PC + 1) + reg_.Y;
		break;

	case NESCPUOpAddrMode::ZEROPAGE:
		addr = comm_->Read8(reg_.PC + 1);
		break;

	case NESCPUOpAddrMode::ZEROPAGE_X:
		addr = (comm_->Read8(reg_.PC + 1) + reg_.X) & 0xFF;
		break;

	case NESCPUOpAddrMode::ZEROPAGE_Y:
		addr = (comm_->Read8(reg_.PC + 1) + reg_.Y) & 0xFF;
		break;

	default:
		// Unhandled addressing mode!
		assert("Unknown addressing mode supplied to WriteOpResult()!" && false);
		return;
	}

	// Assume writing to main memory at addr.
	comm_->Write8(addr, result);
}


NESCPUOpArgInfo NESCPU::ReadOpArgInfo(NESCPUOpAddrMode addrMode)
{
	NESCPUOpArgInfo argInfo(addrMode);

	switch (argInfo.addrMode)
	{
	case NESCPUOpAddrMode::ACCUMULATOR: // Instruction will need to read the register.
	case NESCPUOpAddrMode::IMPLIED: // No operands for implied addr modes.
	case NESCPUOpAddrMode::IMPLIED_BRK:
		break;

	case NESCPUOpAddrMode::IMMEDIATE:
		argInfo.argAddr = reg_.PC + 1;
		break;

	case NESCPUOpAddrMode::RELATIVE:
		// We add an extra 2 to the PC to cover the size of the rest of the instruction.
		// The offset is a signed 2s complement number.
		argInfo.argAddr = reg_.PC + 2 + static_cast<s8>(comm_->Read8(reg_.PC + 1));
		break;

	case NESCPUOpAddrMode::INDIRECT:
		argInfo.argAddr = NESHelper::MemoryIndirectRead16(*comm_, NESHelper::MemoryRead16(*comm_, reg_.PC + 1));
		break;

	case NESCPUOpAddrMode::INDIRECT_X:
		argInfo.argAddr = NESHelper::MemoryIndirectRead16(*comm_, (comm_->Read8(reg_.PC + 1) + reg_.X) & 0xFF);
		break;

	case NESCPUOpAddrMode::INDIRECT_Y:
		argInfo.argAddr = NESHelper::MemoryIndirectRead16(*comm_, comm_->Read8(reg_.PC + 1));
		argInfo.crossedPage = !NESHelper::IsInSamePage(argInfo.argAddr, argInfo.argAddr + reg_.Y);
		argInfo.argAddr += reg_.Y;
		break;

	case NESCPUOpAddrMode::ABSOLUTE:
		argInfo.argAddr = NESHelper::MemoryRead16(*comm_, reg_.PC + 1);
		break;

	case NESCPUOpAddrMode::ABSOLUTE_X:
		argInfo.argAddr = NESHelper::MemoryRead16(*comm_, reg_.PC + 1);
		argInfo.crossedPage = !NESHelper::IsInSamePage(argInfo.argAddr, argInfo.argAddr + reg_.X);
		argInfo.argAddr += reg_.X;
		break;

	case NESCPUOpAddrMode::ABSOLUTE_Y:
		argInfo.argAddr = NESHelper::MemoryRead16(*comm_, reg_.PC + 1);
		argInfo.crossedPage = !NESHelper::IsInSamePage(argInfo.argAddr, argInfo.argAddr + reg_.Y);
		argInfo.argAddr += reg_.Y;
		break;

	case NESCPUOpAddrMode::ZEROPAGE:
		argInfo.argAddr = comm_->Read8(reg_.PC + 1);
		break;

	case NESCPUOpAddrMode::ZEROPAGE_X:
		argInfo.argAddr = (comm_->Read8(reg_.PC + 1) + reg_.X) & 0xFF;
		break;

	case NESCPUOpAddrMode::ZEROPAGE_Y:
		argInfo.argAddr = (comm_->Read8(reg_.PC + 1) + reg_.Y) & 0xFF;
		break;

	default:
		// Unhandled addressing mode!
		assert("Unknown addressing mode supplied to ReadOpArgInfo()!" && false);
		break;
	}

	return argInfo;
}


//void NESCPU::FetchOp()
//{
//	// Fetch the amount of cycles not yet handled by the previous instruction.
//	const auto unhandledCycles = (currentOp_.IsValid() ? currentOp_.opCyclesLeft : 0);
//
//	try
//	{
//		// Get the next opcode.
//		currentOp_ = NESCPUExecutingOpInfo(comm_->Read8(reg_.PC));
//	}
//	catch (const NESMemoryException&)
//	{
//		throw NESCPUExecutionException("Could not read the next opcode for program execution.", reg_);
//	}
//
//	// Get opcode mapping info.
//	assert("Invalid opcode!" && opInfos_[currentOp_.op].opFunc != nullptr);
//
//	// Add the amount of unhandled cycles to the current instruction's cycle count.
//	currentOp_.opCyclesLeft = opInfos_[currentOp_.op].cycleCount + unhandledCycles;
//	currentOp_.opChangedPC = false;
//}
//
//
//void NESCPU::ExecuteOp()
//{
//	// Get arg info for executing instruction.
//	auto argInfo = ReadOpArgInfo(opInfos_[currentOp_.op].addrMode);
//
//	u16 val;
//	if (argInfo.addrMode == NESCPUOpAddrMode::IMMEDIATE)
//		val = comm_->Read8(argInfo.argAddr);
//	else if (argInfo.addrMode == NESCPUOpAddrMode::ACCUMULATOR)
//		val = 0;
//	else
//		val = argInfo.argAddr;
//	//
//	//std::cout << "SP: $" << std::hex << +reg_.SP << ",  " <<  << std::endl;
//	//std::cout << "stack: ";
//	//for (u8 i = 0xFF; i >= 0 && i > reg_.SP; --i)
//	//	std::cout << std::hex << +comm_->Read8(NES_CPU_STACK_START + i) << ", ";
//	//std::cout << std::endl;
//	//
//	//static int a = 0;
//	//if (a > 300000)
//	//std::cout << "Cyc: " << elapsedCycles_ << ", Reg: " << reg_.ToString() << "\t Ins: " << OpAsAsm(opInfos_[currentOp_.op].opName, opInfos_[currentOp_.op].addrMode, val) << std::endl;
//	//++a;
//
//	// Execute instruction.
//	(this->*opInfos_[currentOp_.op].opFunc)(argInfo);
//
//	// Go to the next instruction if the CPU isn't now jammed...
//	if (!currentOp_.opChangedPC && !isJammed_)
//		reg_.PC += GetOpSizeFromAddrMode(opInfos_[currentOp_.op].addrMode);
//}


void NESCPU::ExecuteNextOp()
{
	if (isJammed_ || stallTicksLeft_ > 0)
		return;

	try
	{
		// Get the next opcode.
		currentOp_ = NESCPUExecutingOpInfo(comm_->Read8(reg_.PC));
	}
	catch (const NESMemoryException&)
	{
		throw NESCPUExecutionException("Could not read the next opcode for program execution.", reg_);
	}

	// @TODO Debug!
	static bool testDone = false;
	if (comm_->Read8(0x6000) != 0x80 &&
		comm_->Read8(0x6001) == 0xDE &&
		comm_->Read8(0x6002) == 0xB0 &&
		comm_->Read8(0x6003) == 0x61 &&
		!testDone)
	{
		testDone = true;
		std::cout << "Test status: $" << std::hex << +comm_->Read8(0x6000) << std::endl;
		std::cout << "Message: " << std::endl;
		for (u16 i = 0x6004;; ++i)
		{
			const u8 c = comm_->Read8(i);
			if (c == 0)
				break;

			std::cout << c;
		}
		std::cout << std::endl;
	}
	else if (comm_->Read8(0x6000) == 0x81)
		intReset_ = true;

	// Get opcode mapping info.
	const auto& opMapping = opInfos_[currentOp_.op];
	assert("Invalid opcode!" && opMapping.opFunc != nullptr);

	auto argInfo = ReadOpArgInfo(opMapping.addrMode);
	currentOp_.opCyclesLeft = opMapping.cycleCount;
	currentOp_.opChangedPC = false;

	u16 val;
	if (argInfo.addrMode == NESCPUOpAddrMode::IMMEDIATE)
		val = comm_->Read8(argInfo.argAddr);
	else if (argInfo.addrMode == NESCPUOpAddrMode::ACCUMULATOR)
		val = 0;
	else
		val = argInfo.argAddr;
	//
	//std::cout << "SP: $" << std::hex << +reg_.SP << ",  " <<  << std::endl;
	//std::cout << "stack: ";
	//for (u8 i = 0xFF; i >= 0 && i > reg_.SP; --i)
	//	std::cout << std::hex << +comm_->Read8(NES_CPU_STACK_START + i) << ", ";
	//std::cout << std::endl;
	//
	//static int a = 0;
	//if (a > 300000)
	//std::cout << "Cyc: " << elapsedCycles_ << ", Reg: " << reg_.ToString() << "\t Ins: " << OpAsAsm(opMapping.opName, opMapping.addrMode, val) << std::endl;
	//++a;

	// Execute instruction.
	(this->*opMapping.opFunc)(argInfo);

	// Go to the next instruction if the CPU isn't now jammed...
	if (!currentOp_.opChangedPC && !isJammed_)
		reg_.PC += GetOpSizeFromAddrMode(opMapping.addrMode);
}


NESCPUInterruptType NESCPU::HandleInterrupts()
{
	auto handledInt = NESCPUInterruptType::NONE;

	// Determine which interrupt to handle depending on priority.
	if (intReset_)
		handledInt = NESCPUInterruptType::RESET;
	else if (!isJammed_ && stallTicksLeft_ == 0)
	{
		if (intNmi_) // @TODO: Check for NMI Edge!
			handledInt = NESCPUInterruptType::NMI;
		else if (intIrq_ && !NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_I_BIT))
			handledInt = NESCPUInterruptType::IRQ;
	}

	// Check if we have any interrupts we need to currently handle.
	if (handledInt != NESCPUInterruptType::NONE)
	{
		if (handledInt != NESCPUInterruptType::RESET)
		{
			// If it wasn't a reset, we need to push the next PC and status register (P).
			StackPush16(reg_.PC);
			StackPush8(reg_.GetP());
		}
		else
		{
			// SP and P changed from reset.
			reg_.SP = 0xFA;
			reg_.SetP(0x24);
		}

		switch (handledInt)
		{
		case NESCPUInterruptType::RESET:
			UpdateRegPC(NESHelper::MemoryRead16(*comm_, 0xFFFC));
			intReset_ = false;
			break;

		case NESCPUInterruptType::NMI:
			UpdateRegPC(NESHelper::MemoryRead16(*comm_, 0xFFFA));
			intNmi_ = false;
			break;

		case NESCPUInterruptType::IRQ:
			UpdateRegPC(NESHelper::MemoryRead16(*comm_, 0xFFFE));
			intIrq_ = false;
			break;
		}

		// We interrupted, so make sure the interupt disable flag is set.
		reg_.SetP(NESHelper::SetBit(reg_.GetP(), NES_CPU_REG_P_I_BIT));

		// Interrupts take 7 cycles to execute.
		OpAddCycles(7);
	}

	return handledInt;
}


//void NESCPU::Tick()
//{
//	assert(comm_ != nullptr);
//
//	// Check that the current instruction has finished executing.
//	if (currentOp_.opCyclesLeft == 0 && !isJammed_ && stallTicksLeft_ == 0)
//	{
//		// Check for interrupts - we can fetch next instruction now
//		// if no interrupts were handled.
//		if (HandleInterrupts() == NESCPUInterruptType::NONE)
//		{
//			// If we have yet to fetch any instructions, do so now.
//			if (!currentOp_.IsValid())
//				FetchOp();
//			else
//			{
//				// Execute the instruction we previously fetched.
//				ExecuteOp();
//				FetchOp();
//			}
//		}
//	}
//
//	// @TODO Debug!
//	static bool testDone = false;
//	if (comm_->Read8(0x6000) != 0x80 &&
//		comm_->Read8(0x6001) == 0xDE &&
//		comm_->Read8(0x6002) == 0xB0 &&
//		comm_->Read8(0x6003) == 0x61 &&
//		!testDone)
//	{
//		testDone = true;
//		std::cout << "Test status: $" << std::hex << +comm_->Read8(0x6000) << std::endl;
//		std::cout << "Message: " << std::endl;
//		for (u16 i = 0x6004;; ++i)
//		{
//			const u8 c = comm_->Read8(i);
//			if (c == 0)
//				break;
//
//			std::cout << c;
//		}
//		std::cout << std::endl;
//	}
//	else if (comm_->Read8(0x6000) == 0x81)
//		intReset_ = true;
//
//	++elapsedCycles_;
//	if (currentOp_.opCyclesLeft != 0)
//		--currentOp_.opCyclesLeft;
//	if (stallTicksLeft_ != 0)
//		--stallTicksLeft_;
//}


void NESCPU::Tick()
{
	assert(comm_ != nullptr);

	// Check that the current instruction has finished executing.
	if (currentOp_.opCyclesLeft == 0)
	{
		// Check for interrupts - we can execute next instruction now
		// if no interrupts were handled.
		if (HandleInterrupts() == NESCPUInterruptType::NONE)
			ExecuteNextOp();
	}

	++elapsedCycles_;
	if (currentOp_.opCyclesLeft != 0)
		--currentOp_.opCyclesLeft;
	if (stallTicksLeft_ != 0)
		--stallTicksLeft_;
}