#pragma once

#include <unordered_map>
#include <stdexcept>

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

	/* Processor Status (P) */
	u8 P;
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
* Sorted by interrupt priority (0 = highest, 1 = medium, 2 = lowest).
*/
enum class NESCPUInterrupt
{
	RESET, /* RESET interrupt - highest priority. */
	NMI, /* NMI interrupt - medium priority. */
	IRQ /* IRQ/BRK interrupt - lowest priority. */
};

/**
* Enum containing the different addressing modes of operands.
*/
enum class NESCPUOpAddressingMode
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
	UNKNOWN
};

/**
* Allows static constructor mimic to register opcode mappings.
*/
struct NESCPUStaticInit
{
	NESCPUStaticInit();
};

// This needs to be forward-declared so that we can typedef NESOpFuncPointer.
class NESCPU;

// Typedef for a basic opcode executing function.
typedef void (NESCPU::*NESOpFuncPointer)();

/**
* Struct containing opcode function mapping info.
*/
struct NESCPUOpInfo
{
	const NESOpFuncPointer opFunc;
	const NESCPUOpAddressingMode addrMode;
	const int cycleCount;

	NESCPUOpInfo(NESOpFuncPointer opFunc, NESCPUOpAddressingMode addrMode, int cycleCount) :
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
	std::unordered_map<u8, NESCPUOpInfo>::const_iterator opMappingIt;
	int opCycleCount;
	bool opChangedPC;
};

/**
* Handles emulation of the 6502 2A03 CPU used in the NES.
*/
class NESCPU
{
	friend struct NESCPUStaticInit;

public:
	explicit NESCPU(NESCPUMemoryMapper& mem);
	~NESCPU();

	// Resets the CPU.
	void Reset();

	// Runs the CPU for a number of cycles.
	void Run(int numCycles);

	// Gets the current opcode being executed.
	u8 GetCurrentOpcode() const;

private:
	// Allows init of static stuff such as registration of opcode mapp
	static NESCPUStaticInit staticInit_;

	// Contains mapped opcode info.
	static std::unordered_map<u8, NESCPUOpInfo> opInfos_;

	// Registers an opcode mapping.
	static void RegisterOpMapping(u8 op, NESOpFuncPointer opFunc, NESCPUOpAddressingMode addrMode, int cycleCount);

	// Gets the addressing mode of the specified opcode.
	static NESCPUOpAddressingMode GetOpAddressingMode(u8 op);

	// Returns the size of the opcode's addressing mode in bytes.
	static u16 GetOpSizeFromAddressingMode(NESCPUOpAddressingMode addrMode);

	NESCPURegisters reg_;
	NESCPUMemoryMapper& mem_;
	NESCPUExecutingOpInfo currentOp_;

	// (Re-)Initializes the CPU.
	void Initialize();

	// Updates the Z bit of the P register. Sets to 1 if val is zero. Sets to 0 otherwise.
	inline void UpdateRegZ(u8 val) { NESHelper::EditBit(reg_.P, NES_CPU_REG_P_Z_BIT, val == 0); }

	// Updates the N bit of the P register. Sets to the value of val's 7th bit (sign bit).
	inline void UpdateRegN(u8 val) { reg_.P = (reg_.P & 0x7F) | (val & 0x80); }

	// Updates the P register. Ensures that unused bit D is 1 and unused bit B is 0 regardless of val.
	inline void UpdateRegP(u8 val) { reg_.P = (val & 0xCF) | 0x20; }

	// Updates the PC register. Sets PC to val. currentOpChangedPC_ is set to true so PC is not automatically changed afterwards.
	inline void UpdateRegPC(u16 val) { reg_.PC = val; currentOp_.opChangedPC = true; }

	// Adds the specified amount of extra cycles to the current instruction's execution.
	inline void OpAddCycles(int cycleAmount) { currentOp_.opCycleCount += cycleAmount; }

	/**
	* Reads the value of the next op's immediate argument depending on its addressing mode.
	* Also checks if a page boundary was crossed.
	* Returns the value of the arg as the first member of the pair, and whether or not a page
	* boundary was crossed as second.
	*/
	std::pair<u8, bool> ReadOpArgValue();

	// Writes an op's result to the intended piece of memory / register.
	void WriteOpResult(u8 result);

	// Executes the next opcode at the PC.
	// Can throw NESCPUExecutionException.
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

	// Executes the current op as a branch instruction if shouldBranch is true.
	// Adds 1 to the current op's cycle count if branched to same page, 2 if branched to a different page.
	void ExecuteOpAsBranch(bool shouldBranch, int branchSamePageCycleExtra = 1, int branchDiffPageCycleExtra = 2);

	// Executes an interrupt of the specified type.
	void ExecuteInterrupt(NESCPUInterrupt interruptType);

	/****************************************/
	/****** Instruction Implementation ******/
	/****************************************/

	// Execute Add with Carry (ADC).
	void ExecuteOpADC();

	// Execute AND with Accumulator (AND).
	void ExecuteOpAND();

	// Execute Shift Left One Bit (Memory or Accumulator) (ASL).
	void ExecuteOpASL();

	// Execute Branch on Carry Clear (BCC).
	inline void ExecuteOpBCC() { /* Branch on C = 0 */ ExecuteOpAsBranch(!NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_C_BIT)); }

	// Execute Branch on Carry Set (BCS).
	inline void ExecuteOpBCS() { /* Branch on C = 1 */ ExecuteOpAsBranch(NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_C_BIT)); }

	// Execute Branch on Result Zero (BEQ).
	inline void ExecuteOpBEQ() { /* Branch on Z = 1 */ ExecuteOpAsBranch(NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_Z_BIT)); }

	// Execute Test Bits in Memory with Accumulator (BIT).
	void ExecuteOpBIT();

	// Execute Branch on Result Minus (BMI).
	inline void ExecuteOpBMI() { /* Branch on N = 1 */ ExecuteOpAsBranch(NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_N_BIT)); }

	// Execute Branch on Result Not Zero (BNE).
	inline void ExecuteOpBNE() { /* Branch on Z = 0 */ ExecuteOpAsBranch(!NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_Z_BIT)); }

	// Execute Branch on Result Plus (BPL).
	inline void ExecuteOpBPL() { /* Branch on N = 0 */ ExecuteOpAsBranch(!NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_N_BIT)); }

	// Execute Force Break (BRK).
	inline void ExecuteOpBRK() { ExecuteInterrupt(NESCPUInterrupt::IRQ); }

	// Execute Branch on Overflow Clear (BVC).
	inline void ExecuteOpBVC() { /* Branch on V = 0 */ ExecuteOpAsBranch(!NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_V_BIT)); }

	// Execute Branch on Overflow Set (BVS).
	inline void ExecuteOpBVS() { /* Branch on V = 1 */ ExecuteOpAsBranch(NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_V_BIT)); }

	// Execute Clear Carry Flag (CLC).
	inline void ExecuteOpCLC() { /* 0 -> C */ NESHelper::ClearBit(reg_.P, NES_CPU_REG_P_C_BIT); }

	// Execute Clear Decimal Mode (CLD).
	inline void ExecuteOpCLD() { /* 0 -> D */ NESHelper::ClearBit(reg_.P, NES_CPU_REG_P_D_BIT); }

	// Execute Clear Interrupt Disable Bit (CLI).
	inline void ExecuteOpCLI() { /* 0 -> I */ NESHelper::ClearBit(reg_.P, NES_CPU_REG_P_I_BIT); }

	// Execute Clear Overflow Flag (CLV).
	inline void ExecuteOpCLV() { /* 0 -> V */ NESHelper::ClearBit(reg_.P, NES_CPU_REG_P_V_BIT); }

	// Execute Compare Memory and Accumulator (CMP).
	void ExecuteOpCMP();

	// Execute Compare Memory and Index X (CPX).
	void ExecuteOpCPX();

	// Execute Compare Memory and Index Y (CPY).
	void ExecuteOpCPY();

	// Execute Decrement Memory by One (DEC).
	void ExecuteOpDEC();

	// Execute Decrement Index X by One (DEX).
	void ExecuteOpDEX();

	// Execute Decrement Index Y by One (DEY).
	void ExecuteOpDEY();

	// Execute "Exclusive-Or" Memory with Accumulator (EOR).
	void ExecuteOpEOR();

	// Execute Increment Memory by One (INC).
	void ExecuteOpINC();

	// Execute Increment Index X by One (INX).
	void ExecuteOpINX();

	// Execute Increment Index Y by One (INY).
	void ExecuteOpINY();

	// Execute Jump to New Location (JMP).
	void ExecuteOpJMP();

	// Execute Jump to New Location Saving Return Address (JSR).
	void ExecuteOpJSR();

	// Execute Load Accumulator with Memory (LDA).
	void ExecuteOpLDA();

	// Execute Load Index X with Memory (LDX).
	void ExecuteOpLDX();

	// Execute Load Index Y with Memory (LDY).
	void ExecuteOpLDY();

	// Execute Shift Right One Bit (Memory or Accumulator) (LSR).
	void ExecuteOpLSR();

	// Execute No Operation (Do Nothing) (NOP).
	inline void ExecuteOpNOP() { /* Do nothing. */ }

	// Execute "Or" Memory with Accumulator (ORA).
	void ExecuteOpORA();

	// Execute Push Accumulator to Stack (PHA).
	inline void ExecuteOpPHA() { /* A toS */ StackPush8(reg_.A); }

	// Execute Push Processor Status to Stack (PHP).
	inline void ExecuteOpPHP() { /* P toS */ StackPush8(reg_.P | 0x10); }

	// Execute Pull Accumulator from Stack (PLA).
	void ExecuteOpPLA();

	// Execute Pull Processor Status from Stack (PLP).
	inline void ExecuteOpPLP() { /* P fromS. */ UpdateRegP(StackPull8()); }

	// Execute Rotate One Bit Left (ROL).
	void ExecuteOpROL();

	// Execute Rotate One Bit Right (ROR).
	void ExecuteOpROR();

	// Execute Return from Interrupt (RTI).
	inline void ExecuteOpRTI() { /* P fromS PC fromS */ UpdateRegP(StackPull8()); UpdateRegPC(StackPull16()); }

	// Execute Return from Subroutine (RTS).
	inline void ExecuteOpRTS() { /* PC fromS, PC + 1 -> PC */ UpdateRegPC(StackPull16() + 1); }

	// Execute Subtract Memory from Accumulator with Borrow (SBC).
	void ExecuteOpSBC();

	// Execute Set Carry Flag (SEC).
	inline void ExecuteOpSEC() { /* 1 -> C */ NESHelper::SetBit(reg_.P, NES_CPU_REG_P_C_BIT); }

	// Execute Set Decimal Mode (SED).
	inline void ExecuteOpSED() { /* 1 -> D */ NESHelper::SetBit(reg_.P, NES_CPU_REG_P_D_BIT); }

	// Execute Set Interrupt Disable Status (SEI).
	inline void ExecuteOpSEI() { /* 1 -> I */ NESHelper::SetBit(reg_.P, NES_CPU_REG_P_I_BIT); }

	// Execute Store Accumulator in Memory (STA).
	void ExecuteOpSTA();

	// Execute Store Index X in Memory (STX).
	void ExecuteOpSTX();

	// Execute Store Index Y in Memory (STY).
	void ExecuteOpSTY();

	// Execute Transfer Accumulator to Index Y (TAY).
	inline void ExecuteOpTAY() { /* A -> X */ reg_.X = reg_.A; }

	// Execute Transfer Accumulator to Index X (TAX).
	inline void ExecuteOpTAX() { /* A -> Y */ reg_.Y = reg_.A; }

	// Execute Transfer Stack Pointer to Index X (TSX).
	inline void ExecuteOpTSX() { /* S -> X */ reg_.X = reg_.SP; }

	// Execute Transfer Index X to Accumulator (TXA).
	inline void ExecuteOpTXA() { /* X -> A */ reg_.A = reg_.X; }

	// Execute Transfer Index X to Stack Pointer (TXS).
	inline void ExecuteOpTXS() { /* X -> S */ reg_.SP = reg_.X; }

	// Execute Transfer Index Y to Accumulator (TYA).
	inline void ExecuteOpTYA() { /* Y -> A */ reg_.A = reg_.Y; }
};