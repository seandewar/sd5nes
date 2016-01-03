#include "NESCPU.h"

#include <cassert>
#include <sstream>
#include <iostream> // @TODO Debug?

#include "NESMMC.h"


NESCPUMemoryMapper::NESCPUMemoryMapper(NESMemCPURAM& ram, NESPPU& ppu, NESMMC* mmc) :
ram_(ram),
ppu_(ppu),
mmc_(mmc)
{
	assert(mmc_ != nullptr);
}


NESCPUMemoryMapper::~NESCPUMemoryMapper()
{
}


NESPPURegisterType NESCPUMemoryMapper::GetPPURegister(u16 realAddr)
{
	switch (realAddr)
	{
	case 0x2000:
		return NESPPURegisterType::PPUCTRL;

	case 0x2001:
		return NESPPURegisterType::PPUMASK;

	case 0x2002:
		return NESPPURegisterType::PPUSTATUS;

	case 0x2003:
		return NESPPURegisterType::OAMADDR;

	case 0x2004:
		return NESPPURegisterType::OAMDATA;

	case 0x2005:
		return NESPPURegisterType::PPUSCROLL;

	case 0x2006:
		return NESPPURegisterType::PPUADDR;

	case 0x2007:
		return NESPPURegisterType::PPUDATA;

	case 0x4014:
		return NESPPURegisterType::OAMDMA;

	default:
		return NESPPURegisterType::UNKNOWN;
	}
}


void NESCPUMemoryMapper::Write8(u16 addr, u8 val)
{
	if (addr < 0x2000) // RAM
		ram_.Write8(addr & 0x7FF, val);
	else if (addr < 0x4000) // PPU I/O Registers
		ppu_.WriteRegister(GetPPURegister(0x2000 + (addr & 7)), val);
	else if (addr == 0x4014) // PPU I/O OAMDATA Register
		ppu_.WriteRegister(GetPPURegister(0x4014), val);
	else if (addr < 0x4020) // pAPU I/O Registers
		return; // @TODO
	else // Use the MMC
		mmc_->Write8(addr, val);
}


u8 NESCPUMemoryMapper::Read8(u16 addr) const
{
	if (addr < 0x2000) // RAM
		return ram_.Read8(addr & 0x7FF);
	else if (addr < 0x4000) // PPU I/O Registers
		return ppu_.ReadRegister(GetPPURegister(0x2000 + (addr & 7)));
	else if (addr == 0x4014) // PPU I/O OAMDATA Register
		return ppu_.ReadRegister(GetPPURegister(0x4014));
	else if (addr < 0x4020) // pAPU I/O Registers
		return 0; // @TODO
	else // Use the MMC
		return mmc_->Read8(addr);
}


NESCPU::NESCPU(NESCPUMemoryMapper& mem) :
mem_(mem),
elapsedCycles_(0),
isJammed_(false),
intReset_(false),
intNmi_(false),
intIrq_(false)
{
}


NESCPU::~NESCPU()
{
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
	elapsedCycles_ = 0;
	isJammed_ = false;

	intReset_ = true; // Trigger a Reset interrupt.
	intNmi_ = intIrq_ = false;

	reg_.PC = 0xC000;
	reg_.SP = 0xFD;
	reg_.SetP(0x34); // I, B (and bit 5) are set on power.
	reg_.A = reg_.X = reg_.Y = 0;

	// @TODO Memory to power-up state!
}


void NESCPU::Tick()
{
	// @TODO Do nothing if an instruction is still exec.
	// (Check the amount of cycles an instr takes!

	// Check for interrupts.
	const auto handledInt = HandleInterrupts();
	if (handledInt != NESCPUInterruptType::NONE)
	{
		// Interrupts take 7 cycles to execute.
		elapsedCycles_ += 7;
		return;
	}

	// Execute next instruction.
	ExecuteNextOp();
	elapsedCycles_ += currentOp_.opCycleCount;
}


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


NESCPUInterruptType NESCPU::HandleInterrupts()
{
	auto handledInt = NESCPUInterruptType::NONE;

	if (!isJammed_)
	{
		// Handle interrupts while accounting for priority.
		// Only IRQ's check for the interrupt disable flag (I).
		if (intReset_)
		{
			handledInt = NESCPUInterruptType::RESET;

			UpdateRegPC(NESHelper::MemoryRead16(mem_, 0xFFFC));
			reg_.SP = 0xFA; // SP and P changed from reset.
			reg_.SetP(0x24);
			intReset_ = false;
		}
		else if (intNmi_) // @TODO: Check for NMI Edge!
		{
			handledInt = NESCPUInterruptType::NMI;

			UpdateRegPC(NESHelper::MemoryRead16(mem_, 0xFFFA));
			intNmi_ = false;
		}
		else if (intIrq_ && !NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_I_BIT))
		{
			handledInt = NESCPUInterruptType::IRQ;

			UpdateRegPC(NESHelper::MemoryRead16(mem_, 0xFFFE));
			intIrq_ = false;
		}

		if (handledInt != NESCPUInterruptType::NONE)
		{
			// If it wasn't a reset, we need to push the next PC and status register (P).
			if (handledInt != NESCPUInterruptType::RESET)
			{
				StackPush16(reg_.PC + 1);
				StackPush8(reg_.GetP());
			}

			// Make sure the interrupt disable flag is set if we interrupted.
			reg_.SetP(NESHelper::SetBit(reg_.GetP(), NES_CPU_REG_P_I_BIT));
		}
	}

	return handledInt;
}


void NESCPU::ExecuteNextOp()
{
	if (isJammed_)
		return;

	try
	{
		// Get the next opcode.
		currentOp_ = NESCPUExecutingOpInfo(mem_.Read8(reg_.PC));
	}
	catch (const NESMemoryException&)
	{
		throw NESCPUExecutionException("Could not read the next opcode for program execution.", reg_);
	}

	// @TODO Debug!
	if (mem_.Read8(0x6000) != 0x80 &&
		mem_.Read8(0x6001) == 0xDE &&
		mem_.Read8(0x6002) == 0xB0 &&
		mem_.Read8(0x6003) == 0x61)
	{
		std::cout << "Test status: $" << std::hex << +mem_.Read8(0x6000) << std::endl;
		std::cout << "Message: " << std::endl;
		for (u16 i = 0x6004;; ++i)
		{
			const u8 c = mem_.Read8(i);
			if (c == 0)
				break;

			std::cout << c;
		}
		std::cout << std::endl;
		system("pause");
		exit(0);
	}

	// Get opcode mapping info.
	const auto& opMapping = opInfos_[currentOp_.op];
	if (opMapping.opFunc == nullptr)
	{
		std::ostringstream oss;
		oss << "Unknown opcode $" << std::hex << +currentOp_.op << "!";
		throw NESCPUExecutionException(oss.str(), reg_);
	}

	auto argInfo = ReadOpArgInfo(opMapping.addrMode);
	currentOp_.opCycleCount = opMapping.cycleCount;
	currentOp_.opChangedPC = false;

	//u16 val;
	//if (argInfo.addrMode == NESCPUOpAddrMode::IMMEDIATE)
	//	val = mem_.Read8(argInfo.argAddr);
	//else if (argInfo.addrMode == NESCPUOpAddrMode::ACCUMULATOR)
	//	val = 0;
	//else
	//	val = argInfo.argAddr;
	//
	//std::cout << "SP: $" << std::hex << +reg_.SP << ",  " <<  << std::endl;
	//std::cout << "stack: ";
	//for (u8 i = 0xFF; i >= 0 && i > reg_.SP; --i)
	//	std::cout << std::hex << +mem_.Read8(NES_CPU_STACK_START + i) << ", ";
	//std::cout << std::endl;
	//
	//static int a = 0;
	//if (a < 50)
	// std::cout <<"Cyc: " << elapsedCycles_ << ", Reg: " << reg_.ToString() << "\t Ins: " << OpAsAsm(it->second.opName, it->second.addrMode, val) << std::endl;
	//++a;

	// Execute instruction.
	(this->*opMapping.opFunc)(argInfo);

	// Go to the next instruction if the CPU isn't now jammed...
	if (!currentOp_.opChangedPC && !isJammed_)
		reg_.PC += GetOpSizeFromAddrMode(opMapping.addrMode);
}