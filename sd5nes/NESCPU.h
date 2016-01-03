#pragma once

#include <array>
#include <stdexcept>
#include <sstream>

#include "NESTypes.h"
#include "NESHelper.h"
#include "NESCPUConstants.h"
#include "NESCPUOpConstants.h"
#include "NESMemoryConstants.h"
#include "NESMemory.h"
#include "NESGamePak.h"
#include "NESPPU.h"

typedef NESMemory<0x800> NESMemCPURAM;

/**
* Memory module that maps CPU memory.
*/
class NESCPUMemoryMapper : public NESMemoryMapper
{
public:
	NESCPUMemoryMapper(NESMemCPURAM& ram, NESPPU& ppu, NESMMC* mmc);
	virtual ~NESCPUMemoryMapper();

	void Write8(u16 addr, u8 val) override;
	u8 Read8(u16 addr) const override;

private:
	static NESPPURegisterType GetPPURegister(u16 realAddr);

	NESMemCPURAM& ram_;
	NESPPU& ppu_;
	NESMMC* mmc_;
};

/* Positions of the different used status bits inside of the CPU status register.*/
#define NES_CPU_REG_P_C_BIT 0
#define NES_CPU_REG_P_Z_BIT 1
#define NES_CPU_REG_P_I_BIT 2
#define NES_CPU_REG_P_D_BIT 3
#define NES_CPU_REG_P_V_BIT 6
#define NES_CPU_REG_P_N_BIT 7

/**
* Struct containing the registers used by the NES CPU.
*/
struct NESCPURegisters
{
	/* The Program Counter (PC) - pointer to current instruction being executed */
	u16 PC;

	/* The Stack Pointer (SP) */
	u8 SP;

	/* Accumulator (A) and Index Registers X and Y */
	u8 A, X, Y;

	NESCPURegisters() :
		PC(0), SP(0),
		P_(0x20),
		A(0), X(0), Y(0)
	{ }

	/**
	* Sets the current value of Processor Status (P) to val.
	* Makes sure that bit 5 of P is always 1, regardless of val.
	*/
	inline void SetP(u8 val) { P_ = val | 0x20; }

	/* Gets the current value of Processor Status (P) flag. */
	inline u8 GetP() const { return P_; }

	/**
	* Returns a string representation of the value of the registers.
	*/
	inline std::string ToString() const
	{
		std::ostringstream oss;
		oss << "PC: $" << std::hex << PC << ", SP: $" << std::hex << +SP
			<< ", A: $" << std::hex << +A << ", X: $" << std::hex << +X << ", Y: $" << std::hex << +Y
			<< ", P: $" << std::hex << +P_;

		return oss.str();
	}

private:
	/* Processor Status (P) */
	u8 P_;
};

/**
* Exception relating to the execution of CPU instructions.
*/
class NESCPUExecutionException : public std::runtime_error
{
public:
	explicit NESCPUExecutionException(const char* msg, NESCPURegisters reg) :
		std::runtime_error(msg),
		reg_(reg)
	{ }

	explicit NESCPUExecutionException(const std::string& msg, NESCPURegisters reg) :
		std::runtime_error(msg),
		reg_(reg)
	{ }

	virtual ~NESCPUExecutionException()
	{ }

	inline NESCPURegisters GetCPURegisters() const { return reg_; }

private:
	NESCPURegisters reg_;
};

/**
* Enum containing the different interrupts used by the NES CPU.
*/
enum class NESCPUInterruptType
{
	RESET, /* RESET interrupt - highest priority. */
	NMI, /* NMI interrupt - medium priority. */
	IRQ, /* IRQ/BRK interrupt - lowest priority. */
	NONE
};

/**
* Enum containing the different addressing modes of operands.
*/
enum class NESCPUOpAddrMode
{
	IMMEDIATE,
	ZEROPAGE,
	ZEROPAGE_X,
	ZEROPAGE_Y,
	ABSOLUTE,
	ABSOLUTE_X,
	ABSOLUTE_Y,
	INDIRECT,
	INDIRECT_X,
	INDIRECT_Y,
	ACCUMULATOR,
	RELATIVE,
	IMPLIED,
	IMPLIED_BRK,
	UNKNOWN
};

/**
* Struct containing information about the arg being executed.
*/
struct NESCPUOpArgInfo
{
	u16 argAddr;
	bool crossedPage;
	const NESCPUOpAddrMode addrMode;

	NESCPUOpArgInfo(NESCPUOpAddrMode addrMode) :
		addrMode(addrMode),
		argAddr(0),
		crossedPage(false)
	{ }
};

// This needs to be forward-declared so that we can typedef NESOpFuncPointer.
class NESCPU;

// Typedef for a basic opcode executing function.
typedef void (NESCPU::*NESOpFuncPointer)(NESCPUOpArgInfo& argInfo);

/**
* Struct containing opcode function mapping info.
*/
struct NESCPUOpInfo
{
	const std::string opName;
	const bool isOfficialOp;
	const NESOpFuncPointer opFunc;
	const NESCPUOpAddrMode addrMode;
	const int cycleCount;

	NESCPUOpInfo(const std::string& opName, bool isOfficialOp, NESOpFuncPointer opFunc, NESCPUOpAddrMode addrMode, int cycleCount) :
		opName(opName),
		isOfficialOp(isOfficialOp),
		opFunc(opFunc),
		addrMode(addrMode),
		cycleCount(cycleCount)
	{ }
};

/**
* Struct containing information about the currently execution instruction.
*/
struct NESCPUExecutingOpInfo
{
	u8 op;
	int opCycleCount;
	bool opChangedPC;

	NESCPUExecutingOpInfo(u8 op = NES_OP_KIL_IMPLIED1) :
		op(op),
		opCycleCount(0),
		opChangedPC(false)
	{ }
};

/**
* Handles emulation of the 6502 2A03 CPU used in the NES.
*/
class NESCPU
{
public:
	explicit NESCPU(NESCPUMemoryMapper& mem);
	~NESCPU();

	/**
	* Sets the CPU to its power on state and sets a reset interrupt
	* to be triggered for the next CPU tick.
	*/
	void Power();

	/**
	* Sets an interrupt to be executed next frame.
	*/
	void SetInterrupt(NESCPUInterruptType interrupt);

	/**
	* Ticks the CPU. Takes into account the number of cycles each instruction takes to execute.
	* Handles the next available interrupt of highest priority if set and last instruction finished executing.
	* Otherwise, runs the next instruction if last instruction finished executing.
	*/
	void Tick();

	/**
	* Whether or not the CPU is jammed.
	*/
	inline bool IsJammed() const { return isJammed_; }

private:
	// Contains opcode info.
	static std::array<NESCPUOpInfo, 0x100> opInfos_;

	/**
	* Registers an official opcode mapping.
	*/
	static void RegisterOp(const std::string& opName, u8 op, bool isOfficialOp, NESOpFuncPointer opFunc, NESCPUOpAddrMode addrMode, int cycleCount);

	/**
	* Gets the addressing mode of the specified opcode.
	*/
	static NESCPUOpAddrMode GetOpAddrMode(u8 op);

	/**
	* Returns the size of the opcode's addressing mode in bytes.
	*/
	static u16 GetOpSizeFromAddrMode(NESCPUOpAddrMode addrMode);

	/**
	* Return an assembly string representation of an instruction.
	*/
	static std::string OpAsAsm(const std::string& opName, NESCPUOpAddrMode addrMode, u16 val);

	NESCPURegisters reg_;
	NESCPUMemoryMapper& mem_;
	NESCPUExecutingOpInfo currentOp_;
	bool intReset_, intNmi_, intIrq_;
	bool isJammed_;
	unsigned int elapsedCycles_;

	// Updates the Z bit of the P register. Sets to 1 if val is zero. Sets to 0 otherwise.
	inline void UpdateRegZ(u8 val) { reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_Z_BIT, val == 0)); }

	// Updates the N bit of the P register. Sets to the value of val's 7th bit (sign bit).
	inline void UpdateRegN(u8 val) { reg_.SetP((reg_.GetP() & 0x7F) | (val & 0x80)); }

	// Updates the PC register. Sets PC to val. currentOpChangedPC_ is set to true so PC is not automatically changed afterwards.
	inline void UpdateRegPC(u16 val) { reg_.PC = val; currentOp_.opChangedPC = true; }

	// Adds the specified amount of extra cycles to the current instruction's execution.
	inline void OpAddCycles(int cycleAmount) { currentOp_.opCycleCount += cycleAmount; }

	/**
	* Reads the value of the next op's immediate argument depending on its addressing mode.
	* Also checks if a page boundary was crossed.
	* Returns the addr of the arg's value as the first member of the pair, and whether or not a page
	* boundary was crossed as second.
	*/
	inline NESCPUOpArgInfo ReadOpArgInfo(NESCPUOpAddrMode addrMode)
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
			argInfo.argAddr = reg_.PC + 2 + static_cast<s8>(mem_.Read8(reg_.PC + 1));
			break;

		case NESCPUOpAddrMode::INDIRECT:
			argInfo.argAddr = NESHelper::MemoryIndirectRead16(mem_, NESHelper::MemoryRead16(mem_, reg_.PC + 1));
			break;

		case NESCPUOpAddrMode::INDIRECT_X:
			argInfo.argAddr = NESHelper::MemoryIndirectRead16(mem_, (mem_.Read8(reg_.PC + 1) + reg_.X) & 0xFF);
			break;

		case NESCPUOpAddrMode::INDIRECT_Y:
			argInfo.argAddr = NESHelper::MemoryIndirectRead16(mem_, mem_.Read8(reg_.PC + 1));
			argInfo.crossedPage = !NESHelper::IsInSamePage(argInfo.argAddr, argInfo.argAddr + reg_.Y);
			argInfo.argAddr += reg_.Y;
			break;

		case NESCPUOpAddrMode::ABSOLUTE:
			argInfo.argAddr = NESHelper::MemoryRead16(mem_, reg_.PC + 1);
			break;

		case NESCPUOpAddrMode::ABSOLUTE_X:
			argInfo.argAddr = NESHelper::MemoryRead16(mem_, reg_.PC + 1);
			argInfo.crossedPage = !NESHelper::IsInSamePage(argInfo.argAddr, argInfo.argAddr + reg_.X);
			argInfo.argAddr += reg_.X;
			break;

		case NESCPUOpAddrMode::ABSOLUTE_Y:
			argInfo.argAddr = NESHelper::MemoryRead16(mem_, reg_.PC + 1);
			argInfo.crossedPage = !NESHelper::IsInSamePage(argInfo.argAddr, argInfo.argAddr + reg_.Y);
			argInfo.argAddr += reg_.Y;
			break;

		case NESCPUOpAddrMode::ZEROPAGE:
			argInfo.argAddr = mem_.Read8(reg_.PC + 1);
			break;

		case NESCPUOpAddrMode::ZEROPAGE_X:
			argInfo.argAddr = (mem_.Read8(reg_.PC + 1) + reg_.X) & 0xFF;
			break;

		case NESCPUOpAddrMode::ZEROPAGE_Y:
			argInfo.argAddr = (mem_.Read8(reg_.PC + 1) + reg_.Y) & 0xFF;
			break;

		default:
			// Unhandled addressing mode!
			assert("Unknown addressing mode supplied to ReadOpArgInfo()!" && false);
			break;
		}

		return argInfo;
	}

	/**
	* Writes an op's result to the intended piece of memory / register.
	*/
	inline void WriteOpResult(NESCPUOpAddrMode addrMode, u8 result)
	{
		u16 addr;
		switch (addrMode)
		{
		case NESCPUOpAddrMode::ACCUMULATOR:
			reg_.A = result; // Write to accumulator instead.
			return;

		case NESCPUOpAddrMode::INDIRECT_X:
			addr = NESHelper::MemoryIndirectRead16(mem_, (mem_.Read8(reg_.PC + 1) + reg_.X) & 0xFF);
			break;

		case NESCPUOpAddrMode::INDIRECT_Y:
			addr = NESHelper::MemoryIndirectRead16(mem_, mem_.Read8(reg_.PC + 1)) + reg_.Y;
			break;

		case NESCPUOpAddrMode::ABSOLUTE:
			addr = NESHelper::MemoryRead16(mem_, reg_.PC + 1);
			break;

		case NESCPUOpAddrMode::ABSOLUTE_X:
			addr = NESHelper::MemoryRead16(mem_, reg_.PC + 1) + reg_.X;
			break;

		case NESCPUOpAddrMode::ABSOLUTE_Y:
			addr = NESHelper::MemoryRead16(mem_, reg_.PC + 1) + reg_.Y;
			break;

		case NESCPUOpAddrMode::ZEROPAGE:
			addr = mem_.Read8(reg_.PC + 1);
			break;

		case NESCPUOpAddrMode::ZEROPAGE_X:
			addr = (mem_.Read8(reg_.PC + 1) + reg_.X) & 0xFF;
			break;

		case NESCPUOpAddrMode::ZEROPAGE_Y:
			addr = (mem_.Read8(reg_.PC + 1) + reg_.Y) & 0xFF;
			break;

		default:
			// Unhandled addressing mode!
			assert("Unknown addressing mode supplied to WriteOpResult()!" && false);
			return;
		}

		// Assume writing to main memory at addr.
		mem_.Write8(addr, result);
	}

	/**
	* Handles the execution of pending interrupts while accounting for interrupt priority.
	* Returns NESCPUInterruptType::NONE if no interrupt was handled.
	*/
	NESCPUInterruptType HandleInterrupts();

	/**
	* Executes the next opcode at the PC.
	* Can throw NESCPUExecutionException.
	*/
	void ExecuteNextOp();

	// Push 8-bit value onto the stack.
	inline void StackPush8(u8 val) 
	{
		// @NOTE: Some games purposely overflow the stack
		// So there is no need to do any bounds checks.
		mem_.Write8(NES_CPU_STACK_START + (reg_.SP--), val);
	}

	// Push 16-bit value onto the stack.
	inline void StackPush16(u16 val)
	{
		// Push most-significant byte first, then the least.
		StackPush8((val & 0xFF00) >> 8);
		StackPush8(val & 0xFF);
	}

	// Pull 8-bit value from the stack.
	inline u8 StackPull8()
	{
		// @NOTE: Some games purposely underflow the stack
		// So there is no need to do any bounds checks.
		return mem_.Read8(NES_CPU_STACK_START + (++reg_.SP));
	}

	// Pull 16-bit value from the stack.
	inline u16 StackPull16()
	{
		// Pull least-significant byte first, then the most.
		const u8 lo = StackPull8();
		const u8 hi = StackPull8();

		// Convert to 16-bit val.
		return NESHelper::ConvertTo16(hi, lo);
	}

	/****************************************/
	/****** Instruction Implementation ******/
	/****************************************/

	/**
	* Executes the current op as a branch instruction if shouldBranch is true.
	* Adds 1 to the current op's cycle count if branched to same page, 2 if branched to a different page.
	*/
	inline void ExecuteBranch(u16 jumpAddr, bool shouldBranch)
	{
		if (!shouldBranch)
			return;

		// 1 cycle if same page, 2 cycles if different pages.
		OpAddCycles(NESHelper::IsInSamePage(reg_.PC, jumpAddr) ? 1 : 2);

		// Jump to new PC.
		UpdateRegPC(jumpAddr);
	}

	/**
	* Executes an op as an add with carry. Affects N, Z, V and C.
	* Returns the result.
	*/
	inline u8 ExecuteAddWithCarry(u8 argVal)
	{
		// Return A + M + C -> C
		// @NOTE: NES 6502 variant has no BCD mode.
		const u16 res = reg_.A + argVal + (NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_C_BIT) ? 1 : 0);

		// If A and argVal have the same sign, then we have the potential to overflow (when considering 2s complement).
		// If this is the case, and the sign has changed in the result (compare res with either A or argVal), 
		// then we have overflowed. Set V.
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_V_BIT, ((~(reg_.A ^ argVal) & (reg_.A ^ res)) & 0x80) == 0x80));

		// Set carry if we can't represent this number using 8-bits (regardless of 2s complement).
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, res > 0xFF));

		const u8 res8 = res & 0xFF;
		UpdateRegN(res8);
		UpdateRegZ(res8);
		return res8;
	}

	/**
	* Executes an op as a bitwise AND with the accumulator. Affects Z and N.
	* Returns the result.
	*/
	inline u8 ExecuteANDWithA(u8 argVal)
	{
		// Return A AND M
		const u8 res = reg_.A & argVal;

		UpdateRegZ(res);
		UpdateRegN(res);
		return res;
	}

	/**
	* Executes an op as a bitwise OR with the accumulator. Affects Z and N.
	* Returns the result.
	*/
	inline u8 ExecuteORWithA(u8 argVal)
	{
		// A OR M -> A
		const u8 res = argVal | reg_.A;

		UpdateRegZ(res);
		UpdateRegN(res);
		return res;
	}

	/**
	* Executes an op as a bitwise EOR with the accumulator. Affects Z and N.
	* Returns the result.
	*/
	inline u8 ExecuteEORWithA(u8 argVal)
	{
		// A EOR M -> A
		const u8 res = reg_.A ^ argVal;

		UpdateRegN(res);
		UpdateRegZ(res);
		return res;
	}

	/**
	* Executes an op as a bitwise rotate left. Affects Z, N and C.
	* Returns the result.
	*/
	inline u8 ExecuteRotateLeft(u8 argVal)
	{
		// C <- [7654321] <- C
		// Shift to the left and append the carry bit in position 0 if set.
		const u16 res = (argVal << 1) | (NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_C_BIT) ? 1 : 0);

		// Set the carry if there is a set bit in position 8 (which will be lost after we shift).
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (res & 0x100) == 0x100));

		const u8 res8 = res & 0xFF;
		UpdateRegZ(res8);
		UpdateRegN(res8);
		return res8;
	}

	/**
	* Executes an op as a bitwise rotate right. Affects Z, N and C.
	* Returns the result.
	*/
	inline u8 ExecuteRotateRight(u8 argVal)
	{
		// C -> [7654321] -> C
		// Append the carry bit to position 8 if it is set.
		const u16 unshiftedRes = argVal | (NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_C_BIT) ? 0x100 : 0);

		// Set the carry bit if there is a set bit in position 0 (which will be lost after we shift).
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (unshiftedRes & 1) == 1));

		// Now we can shift to the right and safetly lose bit 0 (as it is recorded in the carry bit).
		const u8 res = unshiftedRes >> 1;

		UpdateRegZ(res);
		UpdateRegN(res);
		return res;
	}

	/**
	* Executes a left shift. Affects Z, N and C.
	* Returns the result.
	*/
	inline u8 ExecuteShiftLeft(u8 argVal)
	{
		// C <- [76543210] <- 0
		// Shift to the left. Bit now in position 0 should be 0. Bit originally in pos 8 is lost.
		const u8 res = (argVal << 1) & 0xFF;

		// Set carry bit if bit 7 (which was lost after the shift) was originally 1.
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (argVal & 0x80) == 0x80));
		UpdateRegZ(res);
		UpdateRegN(res);
		return res;
	}

	/**
	* Executes a right shift. Affects Z, N and C.
	* Returns the result.
	*/
	inline u8 ExecuteShiftRight(u8 argVal)
	{
		// 0 -> [76543210] -> C
		// Shift to the right. We will lose bit 0 in the process, and bit 7 should become 0.
		const u8 res = argVal >> 1;

		// Set the carry if the original bit 0 (that we lost) was 1.
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (argVal & 1) == 1));
		UpdateRegZ(res);
		UpdateRegN(res);
		return res;
	}

	/**
	* Executes comparison. Affects N, Z and C.
	*/
	inline void ExecuteComparison(u8 argVal, u8 compareWith)
	{
		const u16 res = compareWith - argVal;

		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, res < 0x100));

		const u8 res8 = res & 0xFF;
		UpdateRegN(res8);
		UpdateRegZ(res8);
	}

	// Execute Add with Carry (ADC).
	inline void ExecuteOpADC(NESCPUOpArgInfo& argInfo) 
	{ 
		// A + M + C -> A, C
		// 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		reg_.A = ExecuteAddWithCarry(mem_.Read8(argInfo.argAddr));
	}

	// Execute AND X with Accumulator, then AND with 7 (AHX).
	inline void ExecuteOpAHX(NESCPUOpArgInfo& argInfo)
	{
		WriteOpResult(argInfo.addrMode, reg_.A & reg_.X & 7);
	}

	// Execute AND with Accumulator, Set Carry if Negative (ANC).
	inline void ExecuteOpANC(NESCPUOpArgInfo& argInfo)
	{
		reg_.A = ExecuteANDWithA(mem_.Read8(argInfo.argAddr));
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (reg_.A & 0x80) == 0x80));
	}

	// Execute AND with Accumulator (AND).
	inline void ExecuteOpAND(NESCPUOpArgInfo& argInfo)
	{
		// A AND M -> A
		// 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		reg_.A = ExecuteANDWithA(mem_.Read8(argInfo.argAddr));
	}

	// Execute AND with Accumulator then Rotate Right in Accumulator (ARR).
	inline void ExecuteOpARR(NESCPUOpArgInfo& argInfo)
	{
		// Append the carry bit to position 8 if it is set.
		const u8 argVal = mem_.Read8(argInfo.argAddr);
		const u8 res = (argVal | (NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_C_BIT) ? 0x100 : 0)) >> 1;

		// C is set depending on bit 6 and V depends on bits 5 and 6.
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (res & 0x40) == 0x40));
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_V_BIT, (((res & 0x40) >> 1) ^ (res & 0x20)) == 0x20));

		UpdateRegZ(res);
		UpdateRegN(res);
		reg_.A = res;
	}

	// Execute Shift Left One Bit (Memory or Accumulator) (ASL).
	inline void ExecuteOpASL(NESCPUOpArgInfo& argInfo)
	{
		// C <- [76543210] <- 0
		WriteOpResult(argInfo.addrMode,
			ExecuteShiftLeft(argInfo.addrMode == NESCPUOpAddrMode::ACCUMULATOR ? reg_.A : mem_.Read8(argInfo.argAddr)));
	}

	// Execute AND with Accumulator then Shift Right (ASR).
	inline void ExecuteOpASR(NESCPUOpArgInfo& argInfo)
	{
		reg_.A = ExecuteShiftRight(reg_.A & mem_.Read8(argInfo.argAddr));
	}

	// Execute AND X with A and Store in X then subtract byte from X (without borrow) (AXS).
	inline void ExecuteOpAXS(NESCPUOpArgInfo& argInfo)
	{
		const u16 res = (reg_.X & reg_.A) - mem_.Read8(argInfo.argAddr);

		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, res < 0x100));

		const u8 res8 = res & 0xFF;
		UpdateRegN(res8);
		UpdateRegZ(res8); // Check first 8-bits.
		reg_.X = res8;
	}

	// Execute Branch on Carry Clear (BCC).
	inline void ExecuteOpBCC(NESCPUOpArgInfo& argInfo) { /* Branch on C = 0 */ ExecuteBranch(argInfo.argAddr, !NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_C_BIT)); }

	// Execute Branch on Carry Set (BCS).
	inline void ExecuteOpBCS(NESCPUOpArgInfo& argInfo) { /* Branch on C = 1 */ ExecuteBranch(argInfo.argAddr, NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_C_BIT)); }

	// Execute Branch on Result Zero (BEQ).
	inline void ExecuteOpBEQ(NESCPUOpArgInfo& argInfo) { /* Branch on Z = 1 */ ExecuteBranch(argInfo.argAddr, NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_Z_BIT)); }

	// Execute Test Bits in Memory with Accumulator (BIT).
	inline void ExecuteOpBIT(NESCPUOpArgInfo& argInfo)
	{
		// A /\ M, M7 -> N, M6 -> V
		const u8 argVal = mem_.Read8(argInfo.argAddr);

		UpdateRegN(argVal);
		UpdateRegZ(argVal & reg_.A);
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_V_BIT, (argVal & 0x40) == 0x40));
	}

	// Execute Branch on Result Minus (BMI).
	inline void ExecuteOpBMI(NESCPUOpArgInfo& argInfo) { /* Branch on N = 1 */ ExecuteBranch(argInfo.argAddr, NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_N_BIT)); }

	// Execute Branch on Result Not Zero (BNE).
	inline void ExecuteOpBNE(NESCPUOpArgInfo& argInfo) { /* Branch on Z = 0 */ ExecuteBranch(argInfo.argAddr, !NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_Z_BIT)); }

	// Execute Branch on Result Plus (BPL).
	inline void ExecuteOpBPL(NESCPUOpArgInfo& argInfo) { /* Branch on N = 0 */ ExecuteBranch(argInfo.argAddr, !NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_N_BIT)); }

	// Execute Force Break (BRK).
	inline void ExecuteOpBRK(NESCPUOpArgInfo& argInfo)
	{
		// Forced Interrupt PC + 2 toS P toS 
		StackPush16(reg_.PC + 2); // There is a padding byte after the opcode, hence the +2.
		StackPush8(reg_.GetP() | 0x10); // Make sure bit 5 is set on the copy we push.
		reg_.SetP(NESHelper::SetBit(reg_.GetP(), NES_CPU_REG_P_I_BIT));
		
		UpdateRegPC(NESHelper::MemoryRead16(mem_, 0xFFFE));
	}

	// Execute Branch on Overflow Clear (BVC).
	inline void ExecuteOpBVC(NESCPUOpArgInfo& argInfo) { /* Branch on V = 0 */ ExecuteBranch(argInfo.argAddr, !NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_V_BIT)); }

	// Execute Branch on Overflow Set (BVS).
	inline void ExecuteOpBVS(NESCPUOpArgInfo& argInfo) { /* Branch on V = 1 */ ExecuteBranch(argInfo.argAddr, NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_V_BIT)); }

	// Execute Clear Carry Flag (CLC).
	inline void ExecuteOpCLC(NESCPUOpArgInfo& argInfo) { /* 0 -> C */ reg_.SetP(NESHelper::ClearBit(reg_.GetP(), NES_CPU_REG_P_C_BIT)); }

	// Execute Clear Decimal Mode (CLD).
	inline void ExecuteOpCLD(NESCPUOpArgInfo& argInfo) { /* 0 -> D */ reg_.SetP(NESHelper::ClearBit(reg_.GetP(), NES_CPU_REG_P_D_BIT)); }

	// Execute Clear Interrupt Disable Bit (CLI).
	inline void ExecuteOpCLI(NESCPUOpArgInfo& argInfo) { /* 0 -> I */ reg_.SetP(NESHelper::ClearBit(reg_.GetP(), NES_CPU_REG_P_I_BIT)); }

	// Execute Clear Overflow Flag (CLV).
	inline void ExecuteOpCLV(NESCPUOpArgInfo& argInfo) { /* 0 -> V */ reg_.SetP(NESHelper::ClearBit(reg_.GetP(), NES_CPU_REG_P_V_BIT)); }

	// Execute Compare Memory and Accumulator (CMP).
	inline void ExecuteOpCMP(NESCPUOpArgInfo& argInfo)
	{
		// CMP takes 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		// A - M
		ExecuteComparison(mem_.Read8(argInfo.argAddr), reg_.A);
	}

	// Execute Compare Memory and Index X (CPX).
	inline void ExecuteOpCPX(NESCPUOpArgInfo& argInfo)
	{
		// X - M
		ExecuteComparison(mem_.Read8(argInfo.argAddr), reg_.X);
	}

	// Execute Compare Memory and Index Y (CPY).
	inline void ExecuteOpCPY(NESCPUOpArgInfo& argInfo)
	{
		// Y - M
		ExecuteComparison(mem_.Read8(argInfo.argAddr), reg_.Y);
	}

	// Execute Subtract 1 from Memory (Without Borrow) (DCP).
	inline void ExecuteOpDCP(NESCPUOpArgInfo& argInfo)
	{
		const u8 res = mem_.Read8(argInfo.argAddr) - 1;
		WriteOpResult(argInfo.addrMode, res);

		ExecuteComparison(mem_.Read8(argInfo.argAddr), reg_.A);
	}

	// Execute Decrement Memory by One (DEC).
	inline void ExecuteOpDEC(NESCPUOpArgInfo& argInfo)
	{
		// M - 1 -> M
		const u8 res = mem_.Read8(argInfo.argAddr) - 1;
		WriteOpResult(argInfo.addrMode, res);

		UpdateRegN(res);
		UpdateRegZ(res);
	}

	// Execute Decrement Index X by One (DEX).
	inline void ExecuteOpDEX(NESCPUOpArgInfo& argInfo)
	{
		// X - 1 -> X
		--reg_.X;
		UpdateRegN(reg_.X);
		UpdateRegZ(reg_.X);
	}

	// Execute Decrement Index Y by One (DEY).
	inline void ExecuteOpDEY(NESCPUOpArgInfo& argInfo)
	{
		// Y - 1 -> Y
		--reg_.Y;
		UpdateRegN(reg_.Y);
		UpdateRegZ(reg_.Y);
	}

	// Execute "Exclusive-Or" Memory with Accumulator (EOR).
	inline void ExecuteOpEOR(NESCPUOpArgInfo& argInfo)
	{
		// A EOR M -> A
		// 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		reg_.A = ExecuteEORWithA(mem_.Read8(argInfo.argAddr));
	}

	// Execute Increment Memory by One (INC).
	inline void ExecuteOpINC(NESCPUOpArgInfo& argInfo)
	{
		// M + 1 -> M
		const u8 res = mem_.Read8(argInfo.argAddr) + 1;
		WriteOpResult(argInfo.addrMode, res);

		UpdateRegN(res);
		UpdateRegZ(res);
	}

	// Execute Increment Index X by One (INX).
	inline void ExecuteOpINX(NESCPUOpArgInfo& argInfo)
	{
		// X + 1 -> X
		++reg_.X;
		UpdateRegN(reg_.X);
		UpdateRegZ(reg_.X);
	}

	// Execute Increment Index Y by One (INY).
	inline void ExecuteOpINY(NESCPUOpArgInfo& argInfo)
	{
		// Y + 1 -> Y
		++reg_.Y;
		UpdateRegN(reg_.Y);
		UpdateRegZ(reg_.Y);
	}

	// Execute Increase Memory by 1, then Subtract Memory from Accumulator (ISC).
	inline void ExecuteOpISC(NESCPUOpArgInfo& argInfo)
	{
		const u8 res = mem_.Read8(argInfo.argAddr) + 1;
		WriteOpResult(argInfo.addrMode, res);

		reg_.A = ExecuteAddWithCarry(~res);
	}

	// Execute Jump to New Location (JMP).
	inline void ExecuteOpJMP(NESCPUOpArgInfo& argInfo)
	{
		// (PC + 1) -> PCL
		// (PC + 2) -> PCH
		UpdateRegPC(argInfo.argAddr);
	}

	// Execute Jump to New Location Saving Return Address (JSR).
	inline void ExecuteOpJSR(NESCPUOpArgInfo& argInfo)
	{
		// PC + 2 toS, (PC + 1) -> PCL
		//             (PC + 2) -> PCH
		StackPush16(reg_.PC + 2);
		UpdateRegPC(argInfo.argAddr);
	}

	// Executes Processor Lock Up (KIL).
	inline void ExecuteOpKIL(NESCPUOpArgInfo& argInfo)
	{
		// Jam the CPU.
		isJammed_ = true;
	}

	// Executes AND Memory with Stack Pointer, Transfer to A, X and SP (LAS).
	inline void ExecuteOpLAS(NESCPUOpArgInfo& argInfo)
	{
		// 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		const u8 res = mem_.Read8(argInfo.argAddr) & reg_.SP;

		reg_.A = reg_.X = reg_.SP = res;
		UpdateRegN(res);
		UpdateRegZ(res);
	}

	// Execute Load Accumulator and X with Memory (LAX).
	inline void ExecuteOpLAX(NESCPUOpArgInfo& argInfo)
	{
		// 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		const u8 res = mem_.Read8(argInfo.argAddr);

		reg_.A = reg_.X = res;
		UpdateRegN(res);
		UpdateRegZ(res);
	}

	// Execute Load Accumulator with Memory (LDA).
	inline void ExecuteOpLDA(NESCPUOpArgInfo& argInfo)
	{
		// LDA takes 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		// M -> A
		const u8 argVal = mem_.Read8(argInfo.argAddr);

		UpdateRegN(argVal);
		UpdateRegZ(argVal);
		reg_.A = argVal;
	}

	// Execute Load Index X with Memory (LDX).
	inline void ExecuteOpLDX(NESCPUOpArgInfo& argInfo)
	{
		// LDX takes 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		// M -> X
		const u8 argVal = mem_.Read8(argInfo.argAddr);

		UpdateRegN(argVal);
		UpdateRegZ(argVal);
		reg_.X = argVal;
	}

	// Execute Load Index Y with Memory (LDY).
	inline void ExecuteOpLDY(NESCPUOpArgInfo& argInfo)
	{
		// LDY takes 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		// M -> Y
		const u8 argVal = mem_.Read8(argInfo.argAddr);

		UpdateRegN(argVal);
		UpdateRegZ(argVal);
		reg_.Y = argVal;
	}

	// Execute Shift Right One Bit (Memory or Accumulator) (LSR).
	inline void ExecuteOpLSR(NESCPUOpArgInfo& argInfo)
	{
		// 0 -> [76543210] -> C
		WriteOpResult(argInfo.addrMode, 
			ExecuteShiftRight(argInfo.addrMode == NESCPUOpAddrMode::ACCUMULATOR ? reg_.A : mem_.Read8(argInfo.argAddr)));
	}

	// Execute No Operation (Do Nothing) (NOP).
	inline void ExecuteOpNOP(NESCPUOpArgInfo& argInfo) 
	{ 
		// Do nothing.

		// Burn an extra cycle if page crossed
		// from use of an unofficial opcode.
		if (argInfo.crossedPage)
			OpAddCycles(1);
	}

	// Execute "Or" Memory with Accumulator (ORA).
	inline void ExecuteOpORA(NESCPUOpArgInfo& argInfo)
	{
		// A OR M -> A
		// 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		reg_.A = ExecuteORWithA(mem_.Read8(argInfo.argAddr));
	}

	// Execute Push Accumulator to Stack (PHA).
	inline void ExecuteOpPHA(NESCPUOpArgInfo& argInfo) { /* A toS */ StackPush8(reg_.A); }

	// Execute Push Processor Status to Stack (PHP).
	inline void ExecuteOpPHP(NESCPUOpArgInfo& argInfo) 
	{ 
		// P toS 
		// Make sure bit 5 is set on the copy we push.
		StackPush8(reg_.GetP() | 0x10); 
	}

	// Execute Pull Accumulator from Stack (PLA).
	inline void ExecuteOpPLA(NESCPUOpArgInfo& argInfo)
	{
		// A fromS.
		const u8 val = StackPull8();

		UpdateRegZ(val);
		UpdateRegN(val);
		reg_.A = val;
	}

	// Execute Pull Processor Status from Stack (PLP).
	inline void ExecuteOpPLP(NESCPUOpArgInfo& argInfo) 
	{ 
		// P fromS.
		// Make sure we unset bit 5 from the copy we fetch.
		reg_.SetP(StackPull8() & 0xEF);
	}

	// Execute Rotate Left, then AND with Accumulator (RLA).
	inline void ExecuteOpRLA(NESCPUOpArgInfo& argInfo)
	{
		// Shift to the left and append the carry bit in position 0 if set.
		const u16 rotateRes = (mem_.Read8(argInfo.argAddr) << 1) | (NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_C_BIT) ? 1 : 0);

		// Set the carry if there is a set bit in position 8 (which will be lost after we shift).
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (rotateRes & 0x100) == 0x100));
		WriteOpResult(argInfo.addrMode, rotateRes & 0xFF);

		reg_.A = ExecuteANDWithA(rotateRes & 0xFF);
	}

	// Execute Rotate Right, then Add to Accumulator (with Carry) (RRA).
	inline void ExecuteOpRRA(NESCPUOpArgInfo& argInfo)
	{
		// Append the carry bit to position 8 if it is set.
		const u16 unshiftedRes = mem_.Read8(argInfo.argAddr) | (NESHelper::IsBitSet(reg_.GetP(), NES_CPU_REG_P_C_BIT) ? 0x100 : 0);

		// Set the carry bit if there is a set bit in position 0 (which will be lost after we shift).
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (unshiftedRes & 1) == 1));

		// Now we can shift to the right and safetly lose bit 0 (as it is recorded in the carry bit).
		const u8 rotateRes = unshiftedRes >> 1;
		WriteOpResult(argInfo.addrMode, rotateRes);

		reg_.A = ExecuteAddWithCarry(rotateRes);
	}

	// Execute Rotate One Bit Left (ROL).
	inline void ExecuteOpROL(NESCPUOpArgInfo& argInfo)
	{
		// C <-[7654321] <- C
		WriteOpResult(argInfo.addrMode, 
			ExecuteRotateLeft(argInfo.addrMode == NESCPUOpAddrMode::ACCUMULATOR ? reg_.A : mem_.Read8(argInfo.argAddr)));
	}

	// Execute Rotate One Bit Right (ROR).
	inline void ExecuteOpROR(NESCPUOpArgInfo& argInfo)
	{
		// C -> [7654321] -> C
		WriteOpResult(argInfo.addrMode,
			ExecuteRotateRight(argInfo.addrMode == NESCPUOpAddrMode::ACCUMULATOR ? reg_.A : mem_.Read8(argInfo.argAddr)));
	}

	// Execute Return from Interrupt (RTI).
	inline void ExecuteOpRTI(NESCPUOpArgInfo& argInfo) 
	{ 
		// P fromS PC fromS
		// Make sure we unset bit 5 in the copy we fetch.
		reg_.SetP(StackPull8() & 0xEF); 
		UpdateRegPC(StackPull16()); 
	}

	// Execute Return from Subroutine (RTS).
	inline void ExecuteOpRTS(NESCPUOpArgInfo& argInfo) { /* PC fromS, PC + 1 -> PC */ UpdateRegPC(StackPull16() + 1); }

	// Execute AND X Register with Accumulator and Store Result in Memory (SAX).
	inline void ExecuteOpSAX(NESCPUOpArgInfo& argInfo)
	{
		WriteOpResult(argInfo.addrMode, reg_.A & reg_.X);
	}

	// Execute Subtract Memory from Accumulator with Borrow (SBC).
	inline void ExecuteOpSBC(NESCPUOpArgInfo& argInfo) 
	{ 
		// A - M - (1 - C) -> A, C
		// 1 extra CPU cycle if a page boundary was crossed.
		if (argInfo.crossedPage)
			OpAddCycles(1);

		// Simply just execute ADC with the bitwise complement of argVal.
		// In 2s complement, this will = (-argVal) - 1.
		// If the carry flag is set, 1 will be added to make it -argVal as intended.
		reg_.A = ExecuteAddWithCarry(~mem_.Read8(argInfo.argAddr));
	}

	// Execute Set Carry Flag (SEC).
	inline void ExecuteOpSEC(NESCPUOpArgInfo& argInfo) { /* 1 -> C */ reg_.SetP(NESHelper::SetBit(reg_.GetP(), NES_CPU_REG_P_C_BIT)); }

	// Execute Set Decimal Mode (SED).
	inline void ExecuteOpSED(NESCPUOpArgInfo& argInfo) { /* 1 -> D */ reg_.SetP(NESHelper::SetBit(reg_.GetP(), NES_CPU_REG_P_D_BIT)); }

	// Execute Set Interrupt Disable Status (SEI).
	inline void ExecuteOpSEI(NESCPUOpArgInfo& argInfo) { /* 1 -> I */ reg_.SetP(NESHelper::SetBit(reg_.GetP(), NES_CPU_REG_P_I_BIT)); }

	// Execute SHX.
	inline void ExecuteOpSHX(NESCPUOpArgInfo& argInfo)
	{
		const u8 res = reg_.X & (argInfo.argAddr >> 8);
		mem_.Write8(NESHelper::ConvertTo16(res, argInfo.argAddr & 0xFF), res);
	}

	// Execute SHY.
	inline void ExecuteOpSHY(NESCPUOpArgInfo& argInfo)
	{
		const u8 res = reg_.Y & (argInfo.argAddr >> 8);
		mem_.Write8(NESHelper::ConvertTo16(res, argInfo.argAddr & 0xFF), res);
	}

	// Execute Shift Right, then EOR Accumulator (SRE).
	inline void ExecuteOpSRE(NESCPUOpArgInfo& argInfo)
	{
		// Shift to the right. We will lose bit 0 in the process, and bit 7 should become 0.
		const u8 argVal = mem_.Read8(argInfo.argAddr);
		const u8 shiftRes = argVal >> 1;

		// Set the carry if the original bit 0 (that we lost) was 1.
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (argVal & 1) == 1));

		WriteOpResult(argInfo.addrMode, shiftRes);
		reg_.A = ExecuteEORWithA(shiftRes);
	}

	// Execute Shift Left, then OR with Accumulator (SLO).
	inline void ExecuteOpSLO(NESCPUOpArgInfo& argInfo)
	{
		// Shift to the left. Bit now in position 0 should be 0. Bit originally in pos 8 is lost.
		const u8 argVal = mem_.Read8(argInfo.argAddr);
		const u8 shiftRes = (argVal << 1) & 0xFF;

		// Set carry bit if bit 7 (which was lost after the shift) was originally 1.
		reg_.SetP(NESHelper::EditBit(reg_.GetP(), NES_CPU_REG_P_C_BIT, (argVal & 0x80) == 0x80));

		WriteOpResult(argInfo.addrMode, shiftRes);
		reg_.A = ExecuteORWithA(shiftRes);
	}

	// Execute Store Accumulator in Memory (STA).
	inline void ExecuteOpSTA(NESCPUOpArgInfo& argInfo) { /* A -> M */ mem_.Write8(argInfo.argAddr, reg_.A); }

	// Execute Store Index X in Memory (STX).
	inline void ExecuteOpSTX(NESCPUOpArgInfo& argInfo) { /* X -> M */ mem_.Write8(argInfo.argAddr, reg_.X); }

	// Execute Store Index Y in Memory (STY).
	inline void ExecuteOpSTY(NESCPUOpArgInfo& argInfo) { /* Y -> M */ mem_.Write8(argInfo.argAddr, reg_.Y); }

	// Execute AND X with Accumulator, Store in SP, then AND SP with High Byte of Arg Addr + 1 (TAS).
	inline void ExecuteOpTAS(NESCPUOpArgInfo& argInfo)
	{
		reg_.SP = (reg_.X & reg_.A) & ((argInfo.argAddr >> 8) + 1);
	}

	// Execute Transfer Accumulator to Index Y (TAY).
	inline void ExecuteOpTAY(NESCPUOpArgInfo& argInfo) 
	{ 
		// A -> Y 
		reg_.Y = reg_.A;
		UpdateRegN(reg_.Y);
		UpdateRegZ(reg_.Y);
	}

	// Execute Transfer Accumulator to Index X (TAX).
	inline void ExecuteOpTAX(NESCPUOpArgInfo& argInfo) 
	{ 
		// A -> X 
		reg_.X = reg_.A;
		UpdateRegN(reg_.X);
		UpdateRegZ(reg_.X);
	}

	// Execute Transfer Stack Pointer to Index X (TSX).
	inline void ExecuteOpTSX(NESCPUOpArgInfo& argInfo) 
	{ 
		// S -> X 
		reg_.X = reg_.SP;
		UpdateRegN(reg_.X);
		UpdateRegZ(reg_.X);
	}

	// Execute Transfer Index X to Accumulator (TXA).
	inline void ExecuteOpTXA(NESCPUOpArgInfo& argInfo) 
	{ 
		// X -> A
		reg_.A = reg_.X; 
		UpdateRegN(reg_.A);
		UpdateRegZ(reg_.A);
	}

	// Execute Transfer Index X to Stack Pointer (TXS).
	inline void ExecuteOpTXS(NESCPUOpArgInfo& argInfo) { /* X -> S */ reg_.SP = reg_.X; }

	// Execute Transfer Index Y to Accumulator (TYA).
	inline void ExecuteOpTYA(NESCPUOpArgInfo& argInfo) 
	{ 
		// Y -> A 
		reg_.A = reg_.Y; 
		UpdateRegN(reg_.A);
		UpdateRegZ(reg_.A);
	}

	// Execute XAA.
	inline void ExecuteOpXAA(NESCPUOpArgInfo& argInfo)
	{
		// This instruction is weird. More info at:
		// http://visual6502.org/wiki/index.php?title=6502_Opcode_8B_%28XAA,_ANE%29
		reg_.A = reg_.X & mem_.Read8(argInfo.argAddr) & (reg_.A | 0xEE);
	}
};