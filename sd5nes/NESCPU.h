#pragma once

#include "NESTypes.h"
#include "NESMemory.h"

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
	UNKNOWN
};

/**
* Handles emulation of the 6502 CPU used in the NES.
*/
class NESCPU
{
public:
	NESCPU(NESMemory& memory);
	~NESCPU();

private:
	NESCPURegisters reg_;
	NESMemory& mem_;

	// Gets the addressing mode of the specified opcode.
	static NESCPUOpAddressingMode GetOpAddressingMode(u8 op);

	// Used to set how many CPU cycles were used by an opcode's execution.
	void SetOpUsedCycles(int cycleAmount);

	// Executes the next opcode at the PC.
	bool ExecuteNextOp();

	// Execute Add with Carry (ADC).
	bool ExecuteOpADC(NESCPUOpAddressingMode addrMode);
};

