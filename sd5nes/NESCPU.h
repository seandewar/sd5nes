#pragma once

#include "NESTypes.h"
#include "NESMemory.h"

#include "NESCPUOpConstants.h"

#include <map>

/**
* Struct containing the registers used by the NES CPU.
*/
struct NESCPURegisters
{
	/* The Program Counter (PC) */
	u16 PC;

	/* The Stack Pointer (SP) */
	u8 SP;

	/* Accumulator (A) and Index Registers X and Y */
	u8 A, X, Y;

	/* Processor Status (P) union - contains the bits in same address space. */
	union
	{
		/* Processor Status (P) */
		u8 P;

		u8 C : 1; /* Carry Flag (C) */
		u8 Z : 1; /* Zero Flag (Z) */
		u8 I : 1; /* Interrupt Disable (I) */
		u8 D : 1; /* Decimal Mode (D) */
		u8 B : 1; /* Break Command (B) */
		u8 PUnused : 1; /* Unused bit */
		u8 V : 1; /* Overflow Flag (V) */
		u8 N : 1; /* Negative Flag (N) */
	};
};

/**
* Enum containing the different interrupts used by the NES CPU.
* Sorted by interrupt priority (0 = highest, 2 = lowest).
*/
enum class NESCPUInterrupt : u8
{
	/* RESET interrupt - highest priority. */
	RESET = 0,

	/* NMI interrupt - medium priority. */
	NMI = 1,

	/* IRQ/BRK interrupt - lowest priority. */
	IRQBRK = 2
};

/**
* Enum containing the different addressing modes of operands.
*/
enum class NESCPUOpAddressingMode
{
	IMMEDIATE,
	ZEROPAGE,
	ZEROPAGE_X,
	ABSOLUTE,
	ABSOLUTE_X,
	ABSOLUTE_Y,
	INDIRECT_X,
	INDIRECT_Y,
	ACCUMULATOR,
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
typedef bool (NESCPU::*NESOpFuncPointer)();

/**
* Struct containing opcode function mapping info.
*/
struct NESCPUOpInfo
{
	NESOpFuncPointer opFunc;
	NESCPUOpAddressingMode addrMode;
	int cycleCount;

	NESCPUOpInfo(NESOpFuncPointer opFunc, NESCPUOpAddressingMode addrMode, int cycleCount) :
		opFunc(opFunc),
		addrMode(addrMode),
		cycleCount(cycleCount)
	{ }
};

/**
* Handles emulation of the 6502 CPU used in the NES.
*/
class NESCPU
{
	friend struct NESCPUStaticInit;

public:
	NESCPU(NESMemory& memory);
	~NESCPU();

	u8 GetCurrentOpcode() const;

private:
	// Allows init of static stuff such as registration of opcode mapp
	static NESCPUStaticInit staticInit_;

	// Contains mapped opcode info.
	static std::map<u8, NESCPUOpInfo> opInfos_;

	// Registers an opcode mapping.
	static void RegisterOpMapping(u8 op, NESOpFuncPointer opFunc, NESCPUOpAddressingMode addrMode, int cycleCount);

	// Gets the addressing mode of the specified opcode.
	static NESCPUOpAddressingMode GetOpAddressingMode(u8 op);

	// Edits outOpSize with the size of the opcode. Returns true on success and false on failure.
	// If false, or outOpSize is null, outOpSize is not modified.
	static bool GetOpSizeFromAddressingMode(NESCPUOpAddressingMode addrMode, int* outOpSize);

	NESCPURegisters reg_;
	NESMemory& mem_;

	u8 currentOp_;
	int currentOpCycleCount_;

	// Reads the value of the next op's immediate argument depending on its addressing mode.
	// Also checks if a page boundary was crossed.
	bool ReadOpArgValue(u8* outVal, bool* outCrossedPageBoundary);

	// Writes an op's result to the intended piece of memory / register.
	bool WriteOpResult(u8 result);

	// Executes the next opcode at the PC.
	bool ExecuteNextOp();

	/****************************************/
	/****** Instruction Implementation ******/
	/****************************************/

	// Execute Add with Carry (ADC).
	bool ExecuteOpADC();

	// Execute AND with Accumulator (AND).
	bool ExecuteOpAND();

	// Execute Shift Left One Bit (Memory or Accumulator) (ASL).
	bool ExecuteOpASL();
};