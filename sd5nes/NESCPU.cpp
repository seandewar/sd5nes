#include "NESCPU.h"

#include <cassert>
#include <sstream>


NESCPUMemoryMapper::NESCPUMemoryMapper(NESMemCPURAM& ram, NESPPURegisters& ppuReg, NESMMC* mmc) :
ram_(ram),
ppuReg_(ppuReg),
mmc_(mmc)
{
}


NESCPUMemoryMapper::~NESCPUMemoryMapper()
{
}


std::pair<INESMemoryInterface*, u16> NESCPUMemoryMapper::GetMapping(u16 addr) const
{
	if (addr < 0x2000)
		return std::make_pair(&ram_, addr & 0x7FF);
	else if (addr < 0x4000) // PPU I/O Registers.
		return std::make_pair(nullptr, (addr & 7) + 0x2000);
	else if (addr < 0x4020) // pAPU I/O Registers.
		return std::make_pair(nullptr, addr);
	else if (addr < 0x6000) // @TODO Expansion ROM.
		return std::make_pair(nullptr, addr);
	else if (addr < 0x8000) // @TODO SRAM.
		return std::make_pair(nullptr, addr);
	else // PRG-ROM Banks - use MMC.
		return std::make_pair(nullptr, addr);
}


void NESCPUMemoryMapper::Write8(u16 addr, u8 val)
{
	const auto mapping = GetMapping(addr);
	if (mapping.first == nullptr)
	{
		if (mapping.second >= 0x8000)
			mmc_->Write8(mapping.second, val);
		else 
		{
			// @TODO
		}

		return;
	}

	NESMemoryMapper::Write8(addr, val);
}


u8 NESCPUMemoryMapper::Read8(u16 addr) const
{
	const auto mapping = GetMapping(addr);
	if (mapping.first == nullptr)
	{
		if (mapping.second >= 0x8000)
			return mmc_->Read8(mapping.second);
		else
		{
			switch (addr)
			{
			case 0x2002:
				const auto oldPpuStat = ppuReg_.PPUSTATUS;
				NESHelper::ClearBit(ppuReg_.PPUSTATUS, NES_PPU_REG_PPUSTATUS_V_BIT); // Clear V-Blank flag on read.
				ppuReg_.PPUADDR = ppuReg_.PPUSCROLL = 0; // Reset $2005 & $2006.
				return oldPpuStat;
			}
			// @TODO
		}
	}

	return NESMemoryMapper::Read8(addr);
}


std::unordered_map<u8, NESCPUOpInfo> NESCPU::opInfos_;
NESCPUStaticInit NESCPU::staticInit_;


void NESCPU::RegisterOpMapping(u8 op, NESOpFuncPointer opFunc, NESCPUOpAddressingMode addrMode, int cycleCount)
{
	assert("Duplicate op mapping entry or unknown addressing mode!" 
		&& (opInfos_.emplace(op, NESCPUOpInfo(opFunc, addrMode, cycleCount)).second)
		&& (addrMode != NESCPUOpAddressingMode::UNKNOWN));
}


NESCPUStaticInit::NESCPUStaticInit()
{
	/****************************************/
	/********* Instruction Mappings *********/
	/****************************************/

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

	// BMI
	NESCPU::RegisterOpMapping(NES_OP_BMI_RELATIVE, &NESCPU::ExecuteOpBMI, NESCPUOpAddressingMode::RELATIVE, 2);

	// BNE
	NESCPU::RegisterOpMapping(NES_OP_BNE_RELATIVE, &NESCPU::ExecuteOpBNE, NESCPUOpAddressingMode::RELATIVE, 2);

	// BPL
	NESCPU::RegisterOpMapping(NES_OP_BPL_RELATIVE, &NESCPU::ExecuteOpBPL, NESCPUOpAddressingMode::RELATIVE, 2);

	// BRK
	NESCPU::RegisterOpMapping(NES_OP_BRK_IMPLIED, &NESCPU::ExecuteOpBRK, NESCPUOpAddressingMode::IMPLIED, 7);

	// BVC
	NESCPU::RegisterOpMapping(NES_OP_BVC_RELATIVE, &NESCPU::ExecuteOpBVC, NESCPUOpAddressingMode::RELATIVE, 2);

	// BVS
	NESCPU::RegisterOpMapping(NES_OP_BVS_RELATIVE, &NESCPU::ExecuteOpBVS, NESCPUOpAddressingMode::RELATIVE, 2);

	// CLC
	NESCPU::RegisterOpMapping(NES_OP_CLC_IMPLIED, &NESCPU::ExecuteOpCLC, NESCPUOpAddressingMode::IMPLIED, 2);

	// CLD
	NESCPU::RegisterOpMapping(NES_OP_CLD_IMPLIED, &NESCPU::ExecuteOpCLD, NESCPUOpAddressingMode::IMPLIED, 2);

	// CLI
	NESCPU::RegisterOpMapping(NES_OP_CLI_IMPLIED, &NESCPU::ExecuteOpCLI, NESCPUOpAddressingMode::IMPLIED, 2);

	// CLV
	NESCPU::RegisterOpMapping(NES_OP_CLV_IMPLIED, &NESCPU::ExecuteOpCLV, NESCPUOpAddressingMode::IMPLIED, 2);

	// CMP
	NESCPU::RegisterOpMapping(NES_OP_CMP_IMMEDIATE, &NESCPU::ExecuteOpCMP, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_CMP_ABSOLUTE, &NESCPU::ExecuteOpCMP, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_CMP_ABSOLUTE_X, &NESCPU::ExecuteOpCMP, NESCPUOpAddressingMode::ABSOLUTE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_CMP_ABSOLUTE_Y, &NESCPU::ExecuteOpCMP, NESCPUOpAddressingMode::ABSOLUTE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_CMP_ZEROPAGE, &NESCPU::ExecuteOpCMP, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_CMP_ZEROPAGE_X, &NESCPU::ExecuteOpCMP, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_CMP_INDIRECT_X, &NESCPU::ExecuteOpCMP, NESCPUOpAddressingMode::INDIRECT_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_CMP_INDIRECT_Y, &NESCPU::ExecuteOpCMP, NESCPUOpAddressingMode::INDIRECT_Y, 5);

	// CPX
	NESCPU::RegisterOpMapping(NES_OP_CPX_IMMEDIATE, &NESCPU::ExecuteOpCPX, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_CPX_ZEROPAGE, &NESCPU::ExecuteOpCPX, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_CPX_ABSOLUTE, &NESCPU::ExecuteOpCPX, NESCPUOpAddressingMode::ABSOLUTE, 4);

	// CPY
	NESCPU::RegisterOpMapping(NES_OP_CPY_IMMEDIATE, &NESCPU::ExecuteOpCPY, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_CPY_ZEROPAGE, &NESCPU::ExecuteOpCPY, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_CPY_ABSOLUTE, &NESCPU::ExecuteOpCPY, NESCPUOpAddressingMode::ABSOLUTE, 4);

	// DEC
	NESCPU::RegisterOpMapping(NES_OP_DEC_ZEROPAGE, &NESCPU::ExecuteOpDEC, NESCPUOpAddressingMode::ZEROPAGE, 5);
	NESCPU::RegisterOpMapping(NES_OP_DEC_ZEROPAGE_X, &NESCPU::ExecuteOpDEC, NESCPUOpAddressingMode::ZEROPAGE_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_DEC_ABSOLUTE, &NESCPU::ExecuteOpDEC, NESCPUOpAddressingMode::ABSOLUTE, 6);
	NESCPU::RegisterOpMapping(NES_OP_DEC_ABSOLUTE_X, &NESCPU::ExecuteOpDEC, NESCPUOpAddressingMode::ABSOLUTE_X, 7);

	// DEX
	NESCPU::RegisterOpMapping(NES_OP_DEX_IMPLIED, &NESCPU::ExecuteOpDEX, NESCPUOpAddressingMode::IMPLIED, 2);

	// DEY
	NESCPU::RegisterOpMapping(NES_OP_DEY_IMPLIED, &NESCPU::ExecuteOpDEY, NESCPUOpAddressingMode::IMPLIED, 2);

	// EOR
	NESCPU::RegisterOpMapping(NES_OP_EOR_IMMEDIATE, &NESCPU::ExecuteOpEOR, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_EOR_ABSOLUTE, &NESCPU::ExecuteOpEOR, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_EOR_ABSOLUTE_X, &NESCPU::ExecuteOpEOR, NESCPUOpAddressingMode::ABSOLUTE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_EOR_ABSOLUTE_Y, &NESCPU::ExecuteOpEOR, NESCPUOpAddressingMode::ABSOLUTE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_EOR_ZEROPAGE, &NESCPU::ExecuteOpEOR, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_EOR_ZEROPAGE_X, &NESCPU::ExecuteOpEOR, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_EOR_INDIRECT_X, &NESCPU::ExecuteOpEOR, NESCPUOpAddressingMode::INDIRECT_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_EOR_INDIRECT_Y, &NESCPU::ExecuteOpEOR, NESCPUOpAddressingMode::INDIRECT_Y, 5);

	// INC
	NESCPU::RegisterOpMapping(NES_OP_INC_ZEROPAGE, &NESCPU::ExecuteOpINC, NESCPUOpAddressingMode::ZEROPAGE, 5);
	NESCPU::RegisterOpMapping(NES_OP_INC_ZEROPAGE_X, &NESCPU::ExecuteOpINC, NESCPUOpAddressingMode::ZEROPAGE_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_INC_ABSOLUTE, &NESCPU::ExecuteOpINC, NESCPUOpAddressingMode::ABSOLUTE, 6);
	NESCPU::RegisterOpMapping(NES_OP_INC_ABSOLUTE_X, &NESCPU::ExecuteOpINC, NESCPUOpAddressingMode::ABSOLUTE_X, 7);

	// INX
	NESCPU::RegisterOpMapping(NES_OP_INX_IMPLIED, &NESCPU::ExecuteOpINX, NESCPUOpAddressingMode::IMPLIED, 2);

	// INY
	NESCPU::RegisterOpMapping(NES_OP_INY_IMPLIED, &NESCPU::ExecuteOpINY, NESCPUOpAddressingMode::IMPLIED, 2);

	// JMP
	NESCPU::RegisterOpMapping(NES_OP_JMP_ABSOLUTE, &NESCPU::ExecuteOpJMP, NESCPUOpAddressingMode::ABSOLUTE, 3);
	NESCPU::RegisterOpMapping(NES_OP_JMP_INDIRECT, &NESCPU::ExecuteOpJMP, NESCPUOpAddressingMode::INDIRECT, 5);

	// JSR
	NESCPU::RegisterOpMapping(NES_OP_JSR_ABSOLUTE, &NESCPU::ExecuteOpJSR, NESCPUOpAddressingMode::ABSOLUTE, 6);

	// LDA
	NESCPU::RegisterOpMapping(NES_OP_LDA_IMMEDIATE, &NESCPU::ExecuteOpLDA, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_LDA_ABSOLUTE, &NESCPU::ExecuteOpLDA, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_LDA_ABSOLUTE_X, &NESCPU::ExecuteOpLDA, NESCPUOpAddressingMode::ABSOLUTE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_LDA_ABSOLUTE_Y, &NESCPU::ExecuteOpLDA, NESCPUOpAddressingMode::ABSOLUTE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_LDA_ZEROPAGE, &NESCPU::ExecuteOpLDA, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_LDA_ZEROPAGE_X, &NESCPU::ExecuteOpLDA, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_LDA_INDIRECT_X, &NESCPU::ExecuteOpLDA, NESCPUOpAddressingMode::INDIRECT_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_LDA_INDIRECT_Y, &NESCPU::ExecuteOpLDA, NESCPUOpAddressingMode::INDIRECT_Y, 5);

	// LDX
	NESCPU::RegisterOpMapping(NES_OP_LDX_IMMEDIATE, &NESCPU::ExecuteOpLDX, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_LDX_ZEROPAGE, &NESCPU::ExecuteOpLDX, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_LDX_ZEROPAGE_Y, &NESCPU::ExecuteOpLDX, NESCPUOpAddressingMode::ZEROPAGE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_LDX_ABSOLUTE, &NESCPU::ExecuteOpLDX, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_LDX_ABSOLUTE_Y, &NESCPU::ExecuteOpLDX, NESCPUOpAddressingMode::ABSOLUTE_Y, 4);

	// LDY
	NESCPU::RegisterOpMapping(NES_OP_LDY_IMMEDIATE, &NESCPU::ExecuteOpLDY, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_LDY_ZEROPAGE, &NESCPU::ExecuteOpLDY, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_LDY_ZEROPAGE_X, &NESCPU::ExecuteOpLDY, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_LDY_ABSOLUTE, &NESCPU::ExecuteOpLDY, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_LDY_ABSOLUTE_X, &NESCPU::ExecuteOpLDY, NESCPUOpAddressingMode::ABSOLUTE_X, 4);

	// LSR
	NESCPU::RegisterOpMapping(NES_OP_LSR_ACCUMULATOR, &NESCPU::ExecuteOpLSR, NESCPUOpAddressingMode::ACCUMULATOR, 2);
	NESCPU::RegisterOpMapping(NES_OP_LSR_ZEROPAGE, &NESCPU::ExecuteOpLSR, NESCPUOpAddressingMode::ZEROPAGE, 5);
	NESCPU::RegisterOpMapping(NES_OP_LSR_ZEROPAGE_X, &NESCPU::ExecuteOpLSR, NESCPUOpAddressingMode::ZEROPAGE_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_LSR_ABSOLUTE, &NESCPU::ExecuteOpLSR, NESCPUOpAddressingMode::ABSOLUTE, 6);
	NESCPU::RegisterOpMapping(NES_OP_LSR_ABSOLUTE_X, &NESCPU::ExecuteOpLSR, NESCPUOpAddressingMode::ABSOLUTE_X, 7);

	// NOP
	NESCPU::RegisterOpMapping(NES_OP_NOP_IMPLIED, &NESCPU::ExecuteOpNOP, NESCPUOpAddressingMode::IMPLIED, 2);

	// ORA
	NESCPU::RegisterOpMapping(NES_OP_ORA_IMMEDIATE, &NESCPU::ExecuteOpORA, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_ORA_ABSOLUTE, &NESCPU::ExecuteOpORA, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_ORA_ABSOLUTE_X, &NESCPU::ExecuteOpORA, NESCPUOpAddressingMode::ABSOLUTE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_ORA_ABSOLUTE_Y, &NESCPU::ExecuteOpORA, NESCPUOpAddressingMode::ABSOLUTE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_ORA_ZEROPAGE, &NESCPU::ExecuteOpORA, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_ORA_ZEROPAGE_X, &NESCPU::ExecuteOpORA, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_ORA_INDIRECT_X, &NESCPU::ExecuteOpORA, NESCPUOpAddressingMode::INDIRECT_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_ORA_INDIRECT_Y, &NESCPU::ExecuteOpORA, NESCPUOpAddressingMode::INDIRECT_Y, 5);

	// PHA
	NESCPU::RegisterOpMapping(NES_OP_PHA_IMPLIED, &NESCPU::ExecuteOpPHA, NESCPUOpAddressingMode::IMPLIED, 3);

	// PHP
	NESCPU::RegisterOpMapping(NES_OP_PHP_IMPLIED, &NESCPU::ExecuteOpPHP, NESCPUOpAddressingMode::IMPLIED, 3);

	// PLA
	NESCPU::RegisterOpMapping(NES_OP_PLA_IMPLIED, &NESCPU::ExecuteOpPLA, NESCPUOpAddressingMode::IMPLIED, 4);

	// PLP
	NESCPU::RegisterOpMapping(NES_OP_PLP_IMPLIED, &NESCPU::ExecuteOpPLP, NESCPUOpAddressingMode::IMPLIED, 4);

	// ROL
	NESCPU::RegisterOpMapping(NES_OP_ROL_ACCUMULATOR, &NESCPU::ExecuteOpROL, NESCPUOpAddressingMode::ACCUMULATOR, 2);
	NESCPU::RegisterOpMapping(NES_OP_ROL_ZEROPAGE, &NESCPU::ExecuteOpROL, NESCPUOpAddressingMode::ZEROPAGE, 5);
	NESCPU::RegisterOpMapping(NES_OP_ROL_ZEROPAGE_X, &NESCPU::ExecuteOpROL, NESCPUOpAddressingMode::ZEROPAGE_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_ROL_ABSOLUTE, &NESCPU::ExecuteOpROL, NESCPUOpAddressingMode::ABSOLUTE, 6);
	NESCPU::RegisterOpMapping(NES_OP_ROL_ABSOLUTE_X, &NESCPU::ExecuteOpROL, NESCPUOpAddressingMode::ABSOLUTE_X, 7);

	// ROR
	NESCPU::RegisterOpMapping(NES_OP_ROR_ACCUMULATOR, &NESCPU::ExecuteOpROR, NESCPUOpAddressingMode::ACCUMULATOR, 2);
	NESCPU::RegisterOpMapping(NES_OP_ROR_ZEROPAGE, &NESCPU::ExecuteOpROR, NESCPUOpAddressingMode::ZEROPAGE, 5);
	NESCPU::RegisterOpMapping(NES_OP_ROR_ZEROPAGE_X, &NESCPU::ExecuteOpROR, NESCPUOpAddressingMode::ZEROPAGE_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_ROR_ABSOLUTE, &NESCPU::ExecuteOpROR, NESCPUOpAddressingMode::ABSOLUTE, 6);
	NESCPU::RegisterOpMapping(NES_OP_ROR_ABSOLUTE_X, &NESCPU::ExecuteOpROR, NESCPUOpAddressingMode::ABSOLUTE_X, 7);

	// RTI
	NESCPU::RegisterOpMapping(NES_OP_RTI_IMPLIED, &NESCPU::ExecuteOpRTI, NESCPUOpAddressingMode::IMPLIED, 6);

	// RTS
	NESCPU::RegisterOpMapping(NES_OP_RTS_IMPLIED, &NESCPU::ExecuteOpRTS, NESCPUOpAddressingMode::IMPLIED, 6);

	// SBC
	NESCPU::RegisterOpMapping(NES_OP_SBC_IMMEDIATE, &NESCPU::ExecuteOpSBC, NESCPUOpAddressingMode::IMMEDIATE, 2);
	NESCPU::RegisterOpMapping(NES_OP_SBC_ABSOLUTE, &NESCPU::ExecuteOpSBC, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_SBC_ABSOLUTE_X, &NESCPU::ExecuteOpSBC, NESCPUOpAddressingMode::ABSOLUTE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_SBC_ABSOLUTE_Y, &NESCPU::ExecuteOpSBC, NESCPUOpAddressingMode::ABSOLUTE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_SBC_ZEROPAGE, &NESCPU::ExecuteOpSBC, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_SBC_ZEROPAGE_X, &NESCPU::ExecuteOpSBC, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_SBC_INDIRECT_X, &NESCPU::ExecuteOpSBC, NESCPUOpAddressingMode::INDIRECT_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_SBC_INDIRECT_Y, &NESCPU::ExecuteOpSBC, NESCPUOpAddressingMode::INDIRECT_Y, 5);

	// SEC
	NESCPU::RegisterOpMapping(NES_OP_SEC_IMPLIED, &NESCPU::ExecuteOpSEC, NESCPUOpAddressingMode::IMPLIED, 2);

	// SED
	NESCPU::RegisterOpMapping(NES_OP_SED_IMPLIED, &NESCPU::ExecuteOpSED, NESCPUOpAddressingMode::IMPLIED, 2);

	// SEI
	NESCPU::RegisterOpMapping(NES_OP_SEI_IMPLIED, &NESCPU::ExecuteOpSEI, NESCPUOpAddressingMode::IMPLIED, 2);

	// STA
	NESCPU::RegisterOpMapping(NES_OP_STA_ABSOLUTE, &NESCPU::ExecuteOpSTA, NESCPUOpAddressingMode::ABSOLUTE, 4);
	NESCPU::RegisterOpMapping(NES_OP_STA_ABSOLUTE_X, &NESCPU::ExecuteOpSTA, NESCPUOpAddressingMode::ABSOLUTE_X, 5);
	NESCPU::RegisterOpMapping(NES_OP_STA_ABSOLUTE_Y, &NESCPU::ExecuteOpSTA, NESCPUOpAddressingMode::ABSOLUTE_Y, 5);
	NESCPU::RegisterOpMapping(NES_OP_STA_ZEROPAGE, &NESCPU::ExecuteOpSTA, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_STA_ZEROPAGE_X, &NESCPU::ExecuteOpSTA, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_STA_INDIRECT_X, &NESCPU::ExecuteOpSTA, NESCPUOpAddressingMode::INDIRECT_X, 6);
	NESCPU::RegisterOpMapping(NES_OP_STA_INDIRECT_Y, &NESCPU::ExecuteOpSTA, NESCPUOpAddressingMode::INDIRECT_Y, 6);

	// STX
	NESCPU::RegisterOpMapping(NES_OP_STX_ZEROPAGE, &NESCPU::ExecuteOpSTX, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_STX_ZEROPAGE_Y, &NESCPU::ExecuteOpSTX, NESCPUOpAddressingMode::ZEROPAGE_Y, 4);
	NESCPU::RegisterOpMapping(NES_OP_STX_ABSOLUTE, &NESCPU::ExecuteOpSTX, NESCPUOpAddressingMode::ABSOLUTE, 4);

	// STY
	NESCPU::RegisterOpMapping(NES_OP_STY_ZEROPAGE, &NESCPU::ExecuteOpSTY, NESCPUOpAddressingMode::ZEROPAGE, 3);
	NESCPU::RegisterOpMapping(NES_OP_STY_ZEROPAGE_X, &NESCPU::ExecuteOpSTY, NESCPUOpAddressingMode::ZEROPAGE_X, 4);
	NESCPU::RegisterOpMapping(NES_OP_STY_ABSOLUTE, &NESCPU::ExecuteOpSTY, NESCPUOpAddressingMode::ABSOLUTE, 4);

	// TAX
	NESCPU::RegisterOpMapping(NES_OP_TAX_IMPLIED, &NESCPU::ExecuteOpTAX, NESCPUOpAddressingMode::IMPLIED, 2);

	// TAY
	NESCPU::RegisterOpMapping(NES_OP_TAY_IMPLIED, &NESCPU::ExecuteOpTAY, NESCPUOpAddressingMode::IMPLIED, 2);

	// TSX
	NESCPU::RegisterOpMapping(NES_OP_TSX_IMPLIED, &NESCPU::ExecuteOpTSX, NESCPUOpAddressingMode::IMPLIED, 2);

	// TXA
	NESCPU::RegisterOpMapping(NES_OP_TXA_IMPLIED, &NESCPU::ExecuteOpTXA, NESCPUOpAddressingMode::IMPLIED, 2);

	// TXS
	NESCPU::RegisterOpMapping(NES_OP_TXS_IMPLIED, &NESCPU::ExecuteOpTXS, NESCPUOpAddressingMode::IMPLIED, 2);

	// TYA
	NESCPU::RegisterOpMapping(NES_OP_TYA_IMPLIED, &NESCPU::ExecuteOpTYA, NESCPUOpAddressingMode::IMPLIED, 2);
}


// Defines variables argVal and crossedPageBoundary.
// @NOTE: Safe to end with a semi-colon after using this macro.
#define OP_READ_ARG() \
		const auto valPair = ReadOpArgValue(); \
		const u8& argVal = valPair.first; \
		const bool crossedPageBoundary = valPair.second; \


NESCPU::NESCPU(NESCPUMemoryMapper& mem) :
mem_(mem)
{
	Initialize();
}


NESCPU::~NESCPU()
{
}


void NESCPU::Reset()
{
	// @TODO
	Initialize();
}


void NESCPU::Initialize()
{
	// Reset current op status.
	currentOp_.op = NES_OP_INVALID;
	currentOp_.opCycleCount = 0;
	currentOp_.opChangedPC = false;

	// Init registers.
	reg_.PC = reg_.SP = reg_.A = reg_.X = reg_.Y;
	reg_.P = 0x20; // Unused status register bit should always be 1.
}


u8 NESCPU::GetCurrentOpcode() const
{
	return currentOp_.op;
}


NESCPUOpAddressingMode NESCPU::GetOpAddressingMode(u8 op)
{
	const auto it = opInfos_.find(op);
	if (it == opInfos_.end())
		return NESCPUOpAddressingMode::UNKNOWN;

	return it->second.addrMode;
}


u16 NESCPU::GetOpSizeFromAddressingMode(NESCPUOpAddressingMode addrMode)
{
	switch (addrMode)
	{
	// 1 byte addressing modes.
	case NESCPUOpAddressingMode::ACCUMULATOR:
	case NESCPUOpAddressingMode::RELATIVE:
	case NESCPUOpAddressingMode::IMPLIED:
		return 1;

	// 2 byte addressing modes.
	case NESCPUOpAddressingMode::IMMEDIATE:
	case NESCPUOpAddressingMode::ZEROPAGE:
	case NESCPUOpAddressingMode::ZEROPAGE_X:
	case NESCPUOpAddressingMode::ZEROPAGE_Y:
	case NESCPUOpAddressingMode::INDIRECT_X:
	case NESCPUOpAddressingMode::INDIRECT_Y:
		return 2;

	// 3 byte addressing modes.
	case NESCPUOpAddressingMode::ABSOLUTE:
	case NESCPUOpAddressingMode::ABSOLUTE_X:
	case NESCPUOpAddressingMode::ABSOLUTE_Y:
	case NESCPUOpAddressingMode::INDIRECT:
		return 3;
	}

	// Unknown addressing mode.
	assert("Unknown addressing mode supplied!" && false);
	return 0;
}


void NESCPU::ExecuteNextOp()
{
	// Get the next opcode.
	try
	{
		currentOp_.op = mem_.Read8(reg_.PC);
	}
	catch (const NESMemoryException&)
	{
		throw NESCPUExecutionException("Could not read the next opcode for program execution.", reg_);
	}

	// Locate the mapping for this opcode.
	const auto it = opInfos_.find(currentOp_.op);
	if (it == opInfos_.end())
	{
		std::ostringstream oss;
		oss << "Unknown opcode: 0x" << std::hex << +currentOp_.op;
		throw NESCPUExecutionException(oss.str(), reg_);
	}

	currentOp_.opMappingIt = it;
	currentOp_.opCycleCount = it->second.cycleCount;
	currentOp_.opChangedPC = false;
	(this->*it->second.opFunc)(); // Execute opcode func.

	// Go to the next instruction.
	if (!currentOp_.opChangedPC)
		reg_.PC += GetOpSizeFromAddressingMode(it->second.addrMode);
}


std::pair<u8, bool> NESCPU::ReadOpArgValue()
{
	u16 addr;
	bool crossedPageBoundary;

	switch (currentOp_.opMappingIt->second.addrMode)
	{
	case NESCPUOpAddressingMode::ACCUMULATOR:
		return std::make_pair(reg_.A, false); // Read from accumulator instead.

	case NESCPUOpAddressingMode::IMMEDIATE:
	case NESCPUOpAddressingMode::RELATIVE:
		crossedPageBoundary = false;
		addr = reg_.PC + 1;
		break;

	case NESCPUOpAddressingMode::ABSOLUTE:
	case NESCPUOpAddressingMode::INDIRECT:
		crossedPageBoundary = false;
		addr = NESHelper::MemoryRead16(mem_, reg_.PC + 1);
		break;

	case NESCPUOpAddressingMode::ABSOLUTE_X:
		addr = NESHelper::MemoryRead16(mem_, reg_.PC + 1);
		crossedPageBoundary = !NESHelper::IsInSamePage(addr, addr + reg_.X);
		addr += reg_.X;
		break;

	case NESCPUOpAddressingMode::ABSOLUTE_Y:
		addr = NESHelper::MemoryRead16(mem_, reg_.PC + 1);
		crossedPageBoundary = !NESHelper::IsInSamePage(addr, addr + reg_.Y);
		addr += reg_.Y;
		break;

	case NESCPUOpAddressingMode::ZEROPAGE:
		addr = mem_.Read8(reg_.PC + 1);
		crossedPageBoundary = false;
		break;

	case NESCPUOpAddressingMode::ZEROPAGE_X:
		crossedPageBoundary = false;
		addr = (mem_.Read8(reg_.PC + 1) + reg_.X); // Will wrap around if X is too big.
		break;

	case NESCPUOpAddressingMode::ZEROPAGE_Y:
		crossedPageBoundary = false;
		addr = (mem_.Read8(reg_.PC + 1) + reg_.Y); // Will wrap around if Y is too big.
		break;

	case NESCPUOpAddressingMode::INDIRECT_X:
		crossedPageBoundary = false;
		addr = NESHelper::MemoryRead16(mem_, mem_.Read8(reg_.PC + 1) + reg_.X); // Will wrap around if X is too big.
		break;

	case NESCPUOpAddressingMode::INDIRECT_Y:
		addr = NESHelper::MemoryRead16(mem_, mem_.Read8(reg_.PC + 1));
		crossedPageBoundary = !NESHelper::IsInSamePage(addr, addr + reg_.Y);
		addr += reg_.Y;
		break;

	default:
		// Unhandled addressing mode!
		assert("Unknown addressing mode supplied!" && false);
		return std::make_pair(0, false);
	}

	// Assume reading from main memory.
	return std::make_pair(mem_.Read8(addr), crossedPageBoundary);
}


void NESCPU::WriteOpResult(u8 result)
{
	u16 addr;
	switch (currentOp_.opMappingIt->second.addrMode)
	{
	case NESCPUOpAddressingMode::ACCUMULATOR:
		reg_.A = result; // Write to accumulator instead.
		return;

	case NESCPUOpAddressingMode::ABSOLUTE:
		addr = NESHelper::MemoryRead16(mem_, reg_.PC + 1);
		break;

	case NESCPUOpAddressingMode::ABSOLUTE_X:
		addr = NESHelper::MemoryRead16(mem_, reg_.PC + 1) + reg_.X;
		break;

	case NESCPUOpAddressingMode::ZEROPAGE:
		addr = mem_.Read8(reg_.PC + 1);
		break;

	case NESCPUOpAddressingMode::ZEROPAGE_X:
		addr = (mem_.Read8(reg_.PC + 1) + reg_.X); // Will wrap around if X is too big.
		break;

	default:
		// Unhandled addressing mode!
		assert("Unknown addressing mode supplied!" && false);
		return;
	}

	// Assume writing to main memory at addr.
	mem_.Write8(addr, result);
}


void NESCPU::ExecuteOpADC()
{
	OP_READ_ARG()

	// ADC takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// A + M + C -> A, C
	// @NOTE: NES 6502 variant has no BCD mode.
	const u16 res = reg_.A + argVal + (NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_C_BIT) ? 1 : 0);

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, res > 0xFF);
	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_V_BIT, ((~(reg_.A ^ argVal) & (reg_.A ^ res)) & 0x80) == 0x80); // Check if the sign has changed due to overflow.
	UpdateRegN(static_cast<u8>(res));
	reg_.A = static_cast<u8>(res);
}


void NESCPU::ExecuteOpAND()
{
	OP_READ_ARG()

	// AND takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// A AND M -> A
	const u8 res = reg_.A & argVal;

	UpdateRegZ(res);
	UpdateRegN(res);
	reg_.A = res;
}


void NESCPU::ExecuteOpASL()
{
	OP_READ_ARG()

	// C <- [76543210] <- 0
	const u8 res = argVal << 1;
	WriteOpResult(res);

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, (argVal & 0x80) == 0x80); // Set carry bit if bit 7 was originally 1.
	UpdateRegZ(res);
	UpdateRegN(res);
}


void NESCPU::ExecuteOpAsBranch(bool shouldBranch, int branchSamePageCycleExtra, int branchDiffPageCycleExtra)
{
	if (!shouldBranch)
		return;

	OP_READ_ARG()

	const u16 jumpPC = reg_.PC + argVal;

	// Check if the branch will cross a page boundary.
	if (!NESHelper::IsInSamePage(reg_.PC, jumpPC))
		currentOp_.opCycleCount += branchDiffPageCycleExtra;
	else
		currentOp_.opCycleCount += branchSamePageCycleExtra;

	UpdateRegPC(jumpPC);
}


void NESCPU::ExecuteOpBIT()
{
	OP_READ_ARG()

	// A /\ M, M7 -> N, M6 -> V
	UpdateRegN(argVal);
	UpdateRegZ(argVal & reg_.A);
	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_V_BIT, (argVal & 0x40) == 0x40);
}


void NESCPU::ExecuteInterrupt(NESCPUInterrupt interruptType)
{
	// @TODO Interrupts take 7 cycles.

	switch (interruptType)
	{
	case NESCPUInterrupt::RESET:
		UpdateRegPC(NESHelper::MemoryRead16(mem_, 0xFFFC)); // Jump to RESET Vector.
		break;

	case NESCPUInterrupt::IRQ:
		if (NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_I_BIT)) // IRQ checks the Interupt Disable flag.
			return;

		UpdateRegPC(NESHelper::MemoryRead16(mem_, 0xFFFE)); // Jump to IRQ Vector.
		break;

	case NESCPUInterrupt::NMI:
		UpdateRegPC(NESHelper::MemoryRead16(mem_, 0xFFFA)); // Jump to NMI Vector.
		break;
	}

	if (interruptType != NESCPUInterrupt::RESET)
	{
		// Forced Interrupt PC + 2 toS P toS
		StackPush16(reg_.PC + 1);
		NESHelper::SetBit(reg_.P, NES_CPU_REG_P_B_BIT); // Set break flag before pushing P.
		StackPush16(reg_.P);
		NESHelper::SetBit(reg_.P, NES_CPU_REG_P_I_BIT);
	}
}


void NESCPU::ExecuteOpCMP()
{
	OP_READ_ARG()

	// CMP takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// A - M
	const u16 res = reg_.A - argVal;

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, res < 0x100);
	UpdateRegN(static_cast<u8>(res));
	UpdateRegZ(res & 0xFF); // Check first 8-bits.
}


void NESCPU::ExecuteOpCPX()
{
	OP_READ_ARG()

	// X - M
	const u16 res = reg_.X - argVal;

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, res < 0x100);
	UpdateRegN(static_cast<u8>(res));
	UpdateRegZ(res & 0xFF); // Check first 8-bits.
}


void NESCPU::ExecuteOpCPY()
{
	OP_READ_ARG()

	// Y - M
	const u16 res = reg_.Y - argVal;

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, res < 0x100);
	UpdateRegN(static_cast<u8>(res));
	UpdateRegZ(res & 0xFF); // Check first 8-bits.
}


void NESCPU::ExecuteOpDEC()
{
	OP_READ_ARG()

	// M - 1 -> M
	const u8 res = argVal - 1;
	WriteOpResult(res);

	UpdateRegN(res);
	UpdateRegZ(res);
}


void NESCPU::ExecuteOpDEX()
{
	// X - 1 -> X
	--reg_.X;
	UpdateRegN(reg_.X);
	UpdateRegZ(reg_.X);
}


void NESCPU::ExecuteOpDEY()
{
	// Y - 1 -> Y
	--reg_.Y;
	UpdateRegN(reg_.Y);
	UpdateRegZ(reg_.Y);
}


void NESCPU::ExecuteOpEOR()
{
	OP_READ_ARG()

	// EOR takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// A EOR M -> A
	const u8 res = reg_.A ^ argVal;

	UpdateRegN(res);
	UpdateRegZ(res);
	reg_.A = res;
}


void NESCPU::ExecuteOpINC()
{
	OP_READ_ARG()
	
	// M + 1 -> M
	const u8 res = argVal + 1;
	WriteOpResult(res);

	UpdateRegN(res);
	UpdateRegZ(res);
}


void NESCPU::ExecuteOpINX()
{
	// X + 1 -> X
	++reg_.X;
	UpdateRegN(reg_.X);
	UpdateRegZ(reg_.X);
}


void NESCPU::ExecuteOpINY()
{
	// Y + 1 -> Y
	++reg_.Y;
	UpdateRegN(reg_.Y);
	UpdateRegZ(reg_.Y);
}


void NESCPU::ExecuteOpJMP()
{
	OP_READ_ARG()

	// (PC + 1) -> PCL
	// (PC + 2) -> PCH
	UpdateRegPC(argVal);
}


void NESCPU::ExecuteOpJSR()
{
	OP_READ_ARG()

	// PC + 2 toS, (PC + 1) -> PCL
	//             (PC + 2) -> PCH
	StackPush16(reg_.PC - 1);
	UpdateRegPC(argVal);
}


void NESCPU::ExecuteOpLDA()
{
	OP_READ_ARG()

	// LDA takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// M -> A
	UpdateRegN(argVal);
	UpdateRegZ(argVal);
	reg_.A = argVal;
}


void NESCPU::ExecuteOpLDX()
{
	OP_READ_ARG()

	// LDX takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// M -> X
	UpdateRegN(argVal);
	UpdateRegZ(argVal);
	reg_.X = argVal;
}


void NESCPU::ExecuteOpLDY()
{
	OP_READ_ARG()

	// LDY takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// M -> Y
	UpdateRegN(argVal);
	UpdateRegZ(argVal);
	reg_.Y = argVal;
}


void NESCPU::ExecuteOpLSR()
{
	OP_READ_ARG()

	// 0 -> [76543210] -> C
	const u8 res = argVal >> 1;
	WriteOpResult(res);

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, (argVal & 1) == 1);
	UpdateRegZ(res);
	UpdateRegN(res);
}


void NESCPU::ExecuteOpORA()
{
	OP_READ_ARG()

	// ORA takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// A OR M -> A
	const u8 res = argVal | reg_.A;

	UpdateRegZ(res);
	UpdateRegN(res);
	reg_.A = res;
}


void NESCPU::ExecuteOpPLA()
{
	// A fromS.
	const u8 val = StackPull8();

	UpdateRegZ(val);
	UpdateRegN(val);
	reg_.A = val;
}


void NESCPU::ExecuteOpPLP()
{
	// P fromS.
	const u8 val = StackPull8();

	UpdateRegZ(val);
	UpdateRegN(val);
	reg_.P = val;
}


void NESCPU::ExecuteOpROL()
{
	OP_READ_ARG()

	// C <- [7654321] <- C
	u16 res = argVal << 1;
	if (NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_C_BIT))
		res |= 1;

	WriteOpResult(static_cast<u8>(res));

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, res > 0xFF);
	UpdateRegZ(static_cast<u8>(res));
	UpdateRegN(static_cast<u8>(res));
}


void NESCPU::ExecuteOpROR()
{
	OP_READ_ARG()

	// C -> [7654321] -> C
	u16 res = argVal;
	if (NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_C_BIT))
		res |= 0x100;

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, (res & 1) == 1);

	res >>= 1;
	WriteOpResult(static_cast<u8>(res));

	UpdateRegZ(static_cast<u8>(res));
	UpdateRegN(static_cast<u8>(res));
}


void NESCPU::ExecuteOpSBC()
{
	OP_READ_ARG()

	// SBC takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOp_.opCycleCount;

	// A - M - C -> A
	const u16 res = reg_.A - argVal - 1 + (NESHelper::IsBitSet(reg_.P, NES_CPU_REG_P_C_BIT) ? 1 : 0);

	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_C_BIT, res < 0x100);
	NESHelper::EditBit(reg_.P, NES_CPU_REG_P_V_BIT, ((~(reg_.A ^ argVal) & (reg_.A ^ res)) & 0x80) == 0x80); // Check if the sign has changed due to overflow.
	UpdateRegN(static_cast<u8>(res));
	UpdateRegZ(static_cast<u8>(res));
	reg_.A = static_cast<u8>(res);
}


void NESCPU::ExecuteOpSTA()
{
	OP_READ_ARG()

	// A -> M
	mem_.Write8(argVal, reg_.A);
}


void NESCPU::ExecuteOpSTX()
{
	OP_READ_ARG()

	// X -> M
	mem_.Write8(argVal, reg_.X);
}


void NESCPU::ExecuteOpSTY()
{
	OP_READ_ARG()

	// Y -> M
	mem_.Write8(argVal, reg_.Y);
}