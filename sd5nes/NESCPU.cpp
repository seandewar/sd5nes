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


std::unordered_map<u8, NESCPUOpInfo> NESCPU::opInfos_;
NESCPUStaticInit NESCPU::staticInit_;


void NESCPU::RegisterOp(const std::string& opName, u8 op, bool isOfficialOp, NESOpFuncPointer opFunc, NESCPUOpAddrMode addrMode, int cycleCount)
{
	assert("Duplicate op mapping entry or unknown addressing mode!"
		&& (opInfos_.emplace(op, NESCPUOpInfo(opName, isOfficialOp, opFunc, addrMode, cycleCount)).second)
		&& (addrMode != NESCPUOpAddrMode::UNKNOWN));
}


// This is needed because Intellisense will give errors
// saying that NESCPU::RegisterOp() is inacessible (which is true for most contexts
// but not true inside of NESCPUStaticInit())!
#ifndef __INTELLISENSE__

/**
* Calls NESCPU::RegisterOp() for an official opcode.
*  Allows omitting of &NESCPU:: and NESCPUOpAddrMode:: from arguments.
*/
#define OP(opName, op, opFunc, addrMode, cycleCount) \
	NESCPU::RegisterOp(opName, op, true, &NESCPU::opFunc, NESCPUOpAddrMode::addrMode, cycleCount) \


/**
* Calls NESCPU::RegisterOp() for an unofficial (illegal) opcode.
* Allows omitting of &NESCPU:: and NESCPUOpAddrMode:: from arguments.
*/
#define UOP(opName, op, opFunc, addrMode, cycleCount) \
	NESCPU::RegisterOp(opName, op, false, &NESCPU::opFunc, NESCPUOpAddrMode::addrMode, cycleCount) \

#else

/**
* Calls NESCPU::RegisterOp() for an official opcode.
*  Allows omitting of &NESCPU:: and NESCPUOpAddrMode:: from arguments.
*/
#define OP(opName, op, opFunc, addrMode, cycleCount)


/**
* Calls NESCPU::RegisterOp() for an unofficial (illegal) opcode.
* Allows omitting of &NESCPU:: and NESCPUOpAddrMode:: from arguments.
*/
#define UOP(opName, op, opFunc, addrMode, cycleCount)

#endif


NESCPUStaticInit::NESCPUStaticInit()
{
	/*************************************************/
	/********* Official Instruction Mappings *********/
	/*************************************************/

	// ADC
	OP(NES_OP_ADC_NAME, NES_OP_ADC_IMMEDIATE, ExecuteOpADC, IMMEDIATE, 2);
	OP(NES_OP_ADC_NAME, NES_OP_ADC_ABSOLUTE, ExecuteOpADC, ABSOLUTE, 4);
	OP(NES_OP_ADC_NAME, NES_OP_ADC_ABSOLUTE_X, ExecuteOpADC, ABSOLUTE_X, 4);
	OP(NES_OP_ADC_NAME, NES_OP_ADC_ABSOLUTE_Y, ExecuteOpADC, ABSOLUTE_Y, 4);
	OP(NES_OP_ADC_NAME, NES_OP_ADC_ZEROPAGE, ExecuteOpADC, ZEROPAGE, 3);
	OP(NES_OP_ADC_NAME, NES_OP_ADC_ZEROPAGE_X, ExecuteOpADC, ZEROPAGE_X, 4);
	OP(NES_OP_ADC_NAME, NES_OP_ADC_INDIRECT_X, ExecuteOpADC, INDIRECT_X, 6);
	OP(NES_OP_ADC_NAME, NES_OP_ADC_INDIRECT_Y, ExecuteOpADC, INDIRECT_Y, 5);

	// AND
	OP(NES_OP_AND_NAME, NES_OP_AND_IMMEDIATE, ExecuteOpAND, IMMEDIATE, 2);
	OP(NES_OP_AND_NAME, NES_OP_AND_ABSOLUTE, ExecuteOpAND, ABSOLUTE, 4);
	OP(NES_OP_AND_NAME, NES_OP_AND_ABSOLUTE_X, ExecuteOpAND, ABSOLUTE_X, 4);
	OP(NES_OP_AND_NAME, NES_OP_AND_ABSOLUTE_Y, ExecuteOpAND, ABSOLUTE_Y, 4);
	OP(NES_OP_AND_NAME, NES_OP_AND_ZEROPAGE, ExecuteOpAND, ZEROPAGE, 3);
	OP(NES_OP_AND_NAME, NES_OP_AND_ZEROPAGE_X, ExecuteOpAND, ZEROPAGE_X, 4);
	OP(NES_OP_AND_NAME, NES_OP_AND_INDIRECT_X, ExecuteOpAND, INDIRECT_X, 6);
	OP(NES_OP_AND_NAME, NES_OP_AND_INDIRECT_Y, ExecuteOpAND, INDIRECT_Y, 5);

	// ASL
	OP(NES_OP_ASL_NAME, NES_OP_ASL_ACCUMULATOR, ExecuteOpASL, ACCUMULATOR, 2);
	OP(NES_OP_ASL_NAME, NES_OP_ASL_ZEROPAGE, ExecuteOpASL, ZEROPAGE, 5);
	OP(NES_OP_ASL_NAME, NES_OP_ASL_ZEROPAGE_X, ExecuteOpASL, ZEROPAGE_X, 6);
	OP(NES_OP_ASL_NAME, NES_OP_ASL_ABSOLUTE, ExecuteOpASL, ABSOLUTE, 6);
	OP(NES_OP_ASL_NAME, NES_OP_ASL_ABSOLUTE_X, ExecuteOpASL, ABSOLUTE_X, 7);

	// BCC
	OP(NES_OP_BCC_NAME, NES_OP_BCC_RELATIVE, ExecuteOpBCC, RELATIVE, 2);

	// BCS
	OP(NES_OP_BCS_NAME, NES_OP_BCS_RELATIVE, ExecuteOpBCS, RELATIVE, 2);

	// BEQ
	OP(NES_OP_BEQ_NAME, NES_OP_BEQ_RELATIVE, ExecuteOpBEQ, RELATIVE, 2);

	// BIT
	OP(NES_OP_BIT_NAME, NES_OP_BIT_ZEROPAGE, ExecuteOpBIT, ZEROPAGE, 3);
	OP(NES_OP_BIT_NAME, NES_OP_BIT_ABSOLUTE, ExecuteOpBIT, ABSOLUTE, 4);

	// BMI
	OP(NES_OP_BMI_NAME, NES_OP_BMI_RELATIVE, ExecuteOpBMI, RELATIVE, 2);

	// BNE
	OP(NES_OP_BNE_NAME, NES_OP_BNE_RELATIVE, ExecuteOpBNE, RELATIVE, 2);

	// BPL
	OP(NES_OP_BPL_NAME, NES_OP_BPL_RELATIVE, ExecuteOpBPL, RELATIVE, 2);

	// BRK
	OP(NES_OP_BRK_NAME, NES_OP_BRK_IMPLIED, ExecuteOpBRK, IMPLIED_BRK, 7);

	// BVC
	OP(NES_OP_BVC_NAME, NES_OP_BVC_RELATIVE, ExecuteOpBVC, RELATIVE, 2);

	// BVS
	OP(NES_OP_BVS_NAME, NES_OP_BVS_RELATIVE, ExecuteOpBVS, RELATIVE, 2);

	// CLC
	OP(NES_OP_CLC_NAME, NES_OP_CLC_IMPLIED, ExecuteOpCLC, IMPLIED, 2);

	// CLD
	OP(NES_OP_CLD_NAME, NES_OP_CLD_IMPLIED, ExecuteOpCLD, IMPLIED, 2);

	// CLI
	OP(NES_OP_CLI_NAME, NES_OP_CLI_IMPLIED, ExecuteOpCLI, IMPLIED, 2);

	// CLV
	OP(NES_OP_CLV_NAME, NES_OP_CLV_IMPLIED, ExecuteOpCLV, IMPLIED, 2);

	// CMP
	OP(NES_OP_CMP_NAME, NES_OP_CMP_IMMEDIATE, ExecuteOpCMP, IMMEDIATE, 2);
	OP(NES_OP_CMP_NAME, NES_OP_CMP_ABSOLUTE, ExecuteOpCMP, ABSOLUTE, 4);
	OP(NES_OP_CMP_NAME, NES_OP_CMP_ABSOLUTE_X, ExecuteOpCMP, ABSOLUTE_X, 4);
	OP(NES_OP_CMP_NAME, NES_OP_CMP_ABSOLUTE_Y, ExecuteOpCMP, ABSOLUTE_Y, 4);
	OP(NES_OP_CMP_NAME, NES_OP_CMP_ZEROPAGE, ExecuteOpCMP, ZEROPAGE, 3);
	OP(NES_OP_CMP_NAME, NES_OP_CMP_ZEROPAGE_X, ExecuteOpCMP, ZEROPAGE_X, 4);
	OP(NES_OP_CMP_NAME, NES_OP_CMP_INDIRECT_X, ExecuteOpCMP, INDIRECT_X, 6);
	OP(NES_OP_CMP_NAME, NES_OP_CMP_INDIRECT_Y, ExecuteOpCMP, INDIRECT_Y, 5);

	// CPX
	OP(NES_OP_CPX_NAME, NES_OP_CPX_IMMEDIATE, ExecuteOpCPX, IMMEDIATE, 2);
	OP(NES_OP_CPX_NAME, NES_OP_CPX_ZEROPAGE, ExecuteOpCPX, ZEROPAGE, 3);
	OP(NES_OP_CPX_NAME, NES_OP_CPX_ABSOLUTE, ExecuteOpCPX, ABSOLUTE, 4);

	// CPY
	OP(NES_OP_CPY_NAME, NES_OP_CPY_IMMEDIATE, ExecuteOpCPY, IMMEDIATE, 2);
	OP(NES_OP_CPY_NAME, NES_OP_CPY_ZEROPAGE, ExecuteOpCPY, ZEROPAGE, 3);
	OP(NES_OP_CPY_NAME, NES_OP_CPY_ABSOLUTE, ExecuteOpCPY, ABSOLUTE, 4);

	// DEC
	OP(NES_OP_DEC_NAME, NES_OP_DEC_ZEROPAGE, ExecuteOpDEC, ZEROPAGE, 5);
	OP(NES_OP_DEC_NAME, NES_OP_DEC_ZEROPAGE_X, ExecuteOpDEC, ZEROPAGE_X, 6);
	OP(NES_OP_DEC_NAME, NES_OP_DEC_ABSOLUTE, ExecuteOpDEC, ABSOLUTE, 6);
	OP(NES_OP_DEC_NAME, NES_OP_DEC_ABSOLUTE_X, ExecuteOpDEC, ABSOLUTE_X, 7);

	// DEX
	OP(NES_OP_DEX_NAME, NES_OP_DEX_IMPLIED, ExecuteOpDEX, IMPLIED, 2);

	// DEY
	OP(NES_OP_DEY_NAME, NES_OP_DEY_IMPLIED, ExecuteOpDEY, IMPLIED, 2);

	// EOR
	OP(NES_OP_EOR_NAME, NES_OP_EOR_IMMEDIATE, ExecuteOpEOR, IMMEDIATE, 2);
	OP(NES_OP_EOR_NAME, NES_OP_EOR_ABSOLUTE, ExecuteOpEOR, ABSOLUTE, 4);
	OP(NES_OP_EOR_NAME, NES_OP_EOR_ABSOLUTE_X, ExecuteOpEOR, ABSOLUTE_X, 4);
	OP(NES_OP_EOR_NAME, NES_OP_EOR_ABSOLUTE_Y, ExecuteOpEOR, ABSOLUTE_Y, 4);
	OP(NES_OP_EOR_NAME, NES_OP_EOR_ZEROPAGE, ExecuteOpEOR, ZEROPAGE, 3);
	OP(NES_OP_EOR_NAME, NES_OP_EOR_ZEROPAGE_X, ExecuteOpEOR, ZEROPAGE_X, 4);
	OP(NES_OP_EOR_NAME, NES_OP_EOR_INDIRECT_X, ExecuteOpEOR, INDIRECT_X, 6);
	OP(NES_OP_EOR_NAME, NES_OP_EOR_INDIRECT_Y, ExecuteOpEOR, INDIRECT_Y, 5);

	// INC
	OP(NES_OP_INC_NAME, NES_OP_INC_ZEROPAGE, ExecuteOpINC, ZEROPAGE, 5);
	OP(NES_OP_INC_NAME, NES_OP_INC_ZEROPAGE_X, ExecuteOpINC, ZEROPAGE_X, 6);
	OP(NES_OP_INC_NAME, NES_OP_INC_ABSOLUTE, ExecuteOpINC, ABSOLUTE, 6);
	OP(NES_OP_INC_NAME, NES_OP_INC_ABSOLUTE_X, ExecuteOpINC, ABSOLUTE_X, 7);

	// INX
	OP(NES_OP_INX_NAME, NES_OP_INX_IMPLIED, ExecuteOpINX, IMPLIED, 2);

	// INY
	OP(NES_OP_INY_NAME, NES_OP_INY_IMPLIED, ExecuteOpINY, IMPLIED, 2);

	// JMP
	OP(NES_OP_JMP_NAME, NES_OP_JMP_ABSOLUTE, ExecuteOpJMP, ABSOLUTE, 3);
	OP(NES_OP_JMP_NAME, NES_OP_JMP_INDIRECT, ExecuteOpJMP, INDIRECT, 5);

	// JSR
	OP(NES_OP_JSR_NAME, NES_OP_JSR_ABSOLUTE, ExecuteOpJSR, ABSOLUTE, 6);

	// LDA
	OP(NES_OP_LDA_NAME, NES_OP_LDA_IMMEDIATE, ExecuteOpLDA, IMMEDIATE, 2);
	OP(NES_OP_LDA_NAME, NES_OP_LDA_ABSOLUTE, ExecuteOpLDA, ABSOLUTE, 4);
	OP(NES_OP_LDA_NAME, NES_OP_LDA_ABSOLUTE_X, ExecuteOpLDA, ABSOLUTE_X, 4);
	OP(NES_OP_LDA_NAME, NES_OP_LDA_ABSOLUTE_Y, ExecuteOpLDA, ABSOLUTE_Y, 4);
	OP(NES_OP_LDA_NAME, NES_OP_LDA_ZEROPAGE, ExecuteOpLDA, ZEROPAGE, 3);
	OP(NES_OP_LDA_NAME, NES_OP_LDA_ZEROPAGE_X, ExecuteOpLDA, ZEROPAGE_X, 4);
	OP(NES_OP_LDA_NAME, NES_OP_LDA_INDIRECT_X, ExecuteOpLDA, INDIRECT_X, 6);
	OP(NES_OP_LDA_NAME, NES_OP_LDA_INDIRECT_Y, ExecuteOpLDA, INDIRECT_Y, 5);

	// LDX
	OP(NES_OP_LDX_NAME, NES_OP_LDX_IMMEDIATE, ExecuteOpLDX, IMMEDIATE, 2);
	OP(NES_OP_LDX_NAME, NES_OP_LDX_ZEROPAGE, ExecuteOpLDX, ZEROPAGE, 3);
	OP(NES_OP_LDX_NAME, NES_OP_LDX_ZEROPAGE_Y, ExecuteOpLDX, ZEROPAGE_Y, 4);
	OP(NES_OP_LDX_NAME, NES_OP_LDX_ABSOLUTE, ExecuteOpLDX, ABSOLUTE, 4);
	OP(NES_OP_LDX_NAME, NES_OP_LDX_ABSOLUTE_Y, ExecuteOpLDX, ABSOLUTE_Y, 4);

	// LDY
	OP(NES_OP_LDY_NAME, NES_OP_LDY_IMMEDIATE, ExecuteOpLDY, IMMEDIATE, 2);
	OP(NES_OP_LDY_NAME, NES_OP_LDY_ZEROPAGE, ExecuteOpLDY, ZEROPAGE, 3);
	OP(NES_OP_LDY_NAME, NES_OP_LDY_ZEROPAGE_X, ExecuteOpLDY, ZEROPAGE_X, 4);
	OP(NES_OP_LDY_NAME, NES_OP_LDY_ABSOLUTE, ExecuteOpLDY, ABSOLUTE, 4);
	OP(NES_OP_LDY_NAME, NES_OP_LDY_ABSOLUTE_X, ExecuteOpLDY, ABSOLUTE_X, 4);

	// LSR
	OP(NES_OP_LSR_NAME, NES_OP_LSR_ACCUMULATOR, ExecuteOpLSR, ACCUMULATOR, 2);
	OP(NES_OP_LSR_NAME, NES_OP_LSR_ZEROPAGE, ExecuteOpLSR, ZEROPAGE, 5);
	OP(NES_OP_LSR_NAME, NES_OP_LSR_ZEROPAGE_X, ExecuteOpLSR, ZEROPAGE_X, 6);
	OP(NES_OP_LSR_NAME, NES_OP_LSR_ABSOLUTE, ExecuteOpLSR, ABSOLUTE, 6);
	OP(NES_OP_LSR_NAME, NES_OP_LSR_ABSOLUTE_X, ExecuteOpLSR, ABSOLUTE_X, 7);

	// NOP
	OP(NES_OP_NOP_NAME, NES_OP_NOP_IMPLIED, ExecuteOpNOP, IMPLIED, 2);

	// ORA
	OP(NES_OP_ORA_NAME, NES_OP_ORA_IMMEDIATE, ExecuteOpORA, IMMEDIATE, 2);
	OP(NES_OP_ORA_NAME, NES_OP_ORA_ABSOLUTE, ExecuteOpORA, ABSOLUTE, 4);
	OP(NES_OP_ORA_NAME, NES_OP_ORA_ABSOLUTE_X, ExecuteOpORA, ABSOLUTE_X, 4);
	OP(NES_OP_ORA_NAME, NES_OP_ORA_ABSOLUTE_Y, ExecuteOpORA, ABSOLUTE_Y, 4);
	OP(NES_OP_ORA_NAME, NES_OP_ORA_ZEROPAGE, ExecuteOpORA, ZEROPAGE, 3);
	OP(NES_OP_ORA_NAME, NES_OP_ORA_ZEROPAGE_X, ExecuteOpORA, ZEROPAGE_X, 4);
	OP(NES_OP_ORA_NAME, NES_OP_ORA_INDIRECT_X, ExecuteOpORA, INDIRECT_X, 6);
	OP(NES_OP_ORA_NAME, NES_OP_ORA_INDIRECT_Y, ExecuteOpORA, INDIRECT_Y, 5);

	// PHA
	OP(NES_OP_PHA_NAME, NES_OP_PHA_IMPLIED, ExecuteOpPHA, IMPLIED, 3);

	// PHP
	OP(NES_OP_PHP_NAME, NES_OP_PHP_IMPLIED, ExecuteOpPHP, IMPLIED, 3);

	// PLA
	OP(NES_OP_PLA_NAME, NES_OP_PLA_IMPLIED, ExecuteOpPLA, IMPLIED, 4);

	// PLP
	OP(NES_OP_PLP_NAME, NES_OP_PLP_IMPLIED, ExecuteOpPLP, IMPLIED, 4);

	// ROL
	OP(NES_OP_ROL_NAME, NES_OP_ROL_ACCUMULATOR, ExecuteOpROL, ACCUMULATOR, 2);
	OP(NES_OP_ROL_NAME, NES_OP_ROL_ZEROPAGE, ExecuteOpROL, ZEROPAGE, 5);
	OP(NES_OP_ROL_NAME, NES_OP_ROL_ZEROPAGE_X, ExecuteOpROL, ZEROPAGE_X, 6);
	OP(NES_OP_ROL_NAME, NES_OP_ROL_ABSOLUTE, ExecuteOpROL, ABSOLUTE, 6);
	OP(NES_OP_ROL_NAME, NES_OP_ROL_ABSOLUTE_X, ExecuteOpROL, ABSOLUTE_X, 7);

	// ROR
	OP(NES_OP_ROR_NAME, NES_OP_ROR_ACCUMULATOR, ExecuteOpROR, ACCUMULATOR, 2);
	OP(NES_OP_ROR_NAME, NES_OP_ROR_ZEROPAGE, ExecuteOpROR, ZEROPAGE, 5);
	OP(NES_OP_ROR_NAME, NES_OP_ROR_ZEROPAGE_X, ExecuteOpROR, ZEROPAGE_X, 6);
	OP(NES_OP_ROR_NAME, NES_OP_ROR_ABSOLUTE, ExecuteOpROR, ABSOLUTE, 6);
	OP(NES_OP_ROR_NAME, NES_OP_ROR_ABSOLUTE_X, ExecuteOpROR, ABSOLUTE_X, 7);

	// RTI
	OP(NES_OP_RTI_NAME, NES_OP_RTI_IMPLIED, ExecuteOpRTI, IMPLIED, 6);

	// RTS
	OP(NES_OP_RTS_NAME, NES_OP_RTS_IMPLIED, ExecuteOpRTS, IMPLIED, 6);

	// SBC
	OP(NES_OP_SBC_NAME, NES_OP_SBC_IMMEDIATE, ExecuteOpSBC, IMMEDIATE, 2);
	OP(NES_OP_SBC_NAME, NES_OP_SBC_ABSOLUTE, ExecuteOpSBC, ABSOLUTE, 4);
	OP(NES_OP_SBC_NAME, NES_OP_SBC_ABSOLUTE_X, ExecuteOpSBC, ABSOLUTE_X, 4);
	OP(NES_OP_SBC_NAME, NES_OP_SBC_ABSOLUTE_Y, ExecuteOpSBC, ABSOLUTE_Y, 4);
	OP(NES_OP_SBC_NAME, NES_OP_SBC_ZEROPAGE, ExecuteOpSBC, ZEROPAGE, 3);
	OP(NES_OP_SBC_NAME, NES_OP_SBC_ZEROPAGE_X, ExecuteOpSBC, ZEROPAGE_X, 4);
	OP(NES_OP_SBC_NAME, NES_OP_SBC_INDIRECT_X, ExecuteOpSBC, INDIRECT_X, 6);
	OP(NES_OP_SBC_NAME, NES_OP_SBC_INDIRECT_Y, ExecuteOpSBC, INDIRECT_Y, 5);

	// SEC
	OP(NES_OP_SEC_NAME, NES_OP_SEC_IMPLIED, ExecuteOpSEC, IMPLIED, 2);

	// SED
	OP(NES_OP_SED_NAME, NES_OP_SED_IMPLIED, ExecuteOpSED, IMPLIED, 2);

	// SEI
	OP(NES_OP_SEI_NAME, NES_OP_SEI_IMPLIED, ExecuteOpSEI, IMPLIED, 2);

	// STA
	OP(NES_OP_STA_NAME, NES_OP_STA_ABSOLUTE, ExecuteOpSTA, ABSOLUTE, 4);
	OP(NES_OP_STA_NAME, NES_OP_STA_ABSOLUTE_X, ExecuteOpSTA, ABSOLUTE_X, 5);
	OP(NES_OP_STA_NAME, NES_OP_STA_ABSOLUTE_Y, ExecuteOpSTA, ABSOLUTE_Y, 5);
	OP(NES_OP_STA_NAME, NES_OP_STA_ZEROPAGE, ExecuteOpSTA, ZEROPAGE, 3);
	OP(NES_OP_STA_NAME, NES_OP_STA_ZEROPAGE_X, ExecuteOpSTA, ZEROPAGE_X, 4);
	OP(NES_OP_STA_NAME, NES_OP_STA_INDIRECT_X, ExecuteOpSTA, INDIRECT_X, 6);
	OP(NES_OP_STA_NAME, NES_OP_STA_INDIRECT_Y, ExecuteOpSTA, INDIRECT_Y, 6);

	// STX
	OP(NES_OP_STX_NAME, NES_OP_STX_ZEROPAGE, ExecuteOpSTX, ZEROPAGE, 3);
	OP(NES_OP_STX_NAME, NES_OP_STX_ZEROPAGE_Y, ExecuteOpSTX, ZEROPAGE_Y, 4);
	OP(NES_OP_STX_NAME, NES_OP_STX_ABSOLUTE, ExecuteOpSTX, ABSOLUTE, 4);

	// STY
	OP(NES_OP_STY_NAME, NES_OP_STY_ZEROPAGE, ExecuteOpSTY, ZEROPAGE, 3);
	OP(NES_OP_STY_NAME, NES_OP_STY_ZEROPAGE_X, ExecuteOpSTY, ZEROPAGE_X, 4);
	OP(NES_OP_STY_NAME, NES_OP_STY_ABSOLUTE, ExecuteOpSTY, ABSOLUTE, 4);

	// TAX
	OP(NES_OP_TAX_NAME, NES_OP_TAX_IMPLIED, ExecuteOpTAX, IMPLIED, 2);

	// TAY
	OP(NES_OP_TAY_NAME, NES_OP_TAY_IMPLIED, ExecuteOpTAY, IMPLIED, 2);

	// TSX
	OP(NES_OP_TSX_NAME, NES_OP_TSX_IMPLIED, ExecuteOpTSX, IMPLIED, 2);

	// TXA
	OP(NES_OP_TXA_NAME, NES_OP_TXA_IMPLIED, ExecuteOpTXA, IMPLIED, 2);

	// TXS
	OP(NES_OP_TXS_NAME, NES_OP_TXS_IMPLIED, ExecuteOpTXS, IMPLIED, 2);

	// TYA
	OP(NES_OP_TYA_NAME, NES_OP_TYA_IMPLIED, ExecuteOpTYA, IMPLIED, 2);

	/***************************************************/
	/********* Unofficial Instruction Mappings *********/
	/***************************************************/

	// ANC
	UOP(NES_OP_ANC_NAME, NES_OP_ANC_IMMEDIATE1, ExecuteOpANC, IMMEDIATE, 2);
	UOP(NES_OP_ANC_NAME, NES_OP_ANC_IMMEDIATE2, ExecuteOpANC, IMMEDIATE, 2);

	// SAX
	UOP(NES_OP_SAX_NAME, NES_OP_SAX_ZEROPAGE, ExecuteOpSAX, ZEROPAGE, 3);
	UOP(NES_OP_SAX_NAME, NES_OP_SAX_ZEROPAGE_Y, ExecuteOpSAX, ZEROPAGE_Y, 4);
	UOP(NES_OP_SAX_NAME, NES_OP_SAX_INDIRECT_X, ExecuteOpSAX, INDIRECT_X, 6);
	UOP(NES_OP_SAX_NAME, NES_OP_SAX_ABSOLUTE, ExecuteOpSAX, ABSOLUTE, 4);

	// ARR
	UOP(NES_OP_ARR_NAME, NES_OP_ARR_IMMEDIATE, ExecuteOpARR, IMMEDIATE, 2);

	// ASR
	UOP(NES_OP_ASR_NAME, NES_OP_ASR_IMMEDIATE, ExecuteOpASR, IMMEDIATE, 2);

	// @NOTE: Testing shows that this is an LAX #i instead?
	// LXA
	// UOP(NES_OP_LXA_NAME, NES_OP_LXA_IMMEDIATE, ExecuteOpLXA, IMMEDIATE, 2);

	// AHX
	UOP(NES_OP_AHX_NAME, NES_OP_AHX_ABSOLUTE_Y, ExecuteOpAHX, ABSOLUTE_Y, 5);
	UOP(NES_OP_AHX_NAME, NES_OP_AHX_INDIRECT_Y, ExecuteOpAHX, INDIRECT_Y, 6);

	// AXS
	UOP(NES_OP_AXS_NAME, NES_OP_AXS_IMMEDIATE, ExecuteOpAXS, IMMEDIATE, 2);

	// DCP
	UOP(NES_OP_DCP_NAME, NES_OP_DCP_ZEROPAGE, ExecuteOpDCP, ZEROPAGE, 5);
	UOP(NES_OP_DCP_NAME, NES_OP_DCP_ZEROPAGE_X, ExecuteOpDCP, ZEROPAGE_X, 6);
	UOP(NES_OP_DCP_NAME, NES_OP_DCP_ABSOLUTE, ExecuteOpDCP, ABSOLUTE, 6);
	UOP(NES_OP_DCP_NAME, NES_OP_DCP_ABSOLUTE_X, ExecuteOpDCP, ABSOLUTE_X, 7);
	UOP(NES_OP_DCP_NAME, NES_OP_DCP_ABSOLUTE_Y, ExecuteOpDCP, ABSOLUTE_Y, 7);
	UOP(NES_OP_DCP_NAME, NES_OP_DCP_INDIRECT_X, ExecuteOpDCP, INDIRECT_X, 8);
	UOP(NES_OP_DCP_NAME, NES_OP_DCP_INDIRECT_Y, ExecuteOpDCP, INDIRECT_Y, 8);

	// DOP (Double NOP)
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE1, ExecuteOpNOP, ZEROPAGE, 3);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE_X1, ExecuteOpNOP, ZEROPAGE_X, 4);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE_X2, ExecuteOpNOP, ZEROPAGE_X, 4);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE2, ExecuteOpNOP, ZEROPAGE, 3);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE_X3, ExecuteOpNOP, ZEROPAGE_X, 4);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE3, ExecuteOpNOP, ZEROPAGE, 3);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE_X4, ExecuteOpNOP, ZEROPAGE_X, 4);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_IMMEDIATE1, ExecuteOpNOP, IMMEDIATE, 2);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_IMMEDIATE2, ExecuteOpNOP, IMMEDIATE, 2);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_IMMEDIATE3, ExecuteOpNOP, IMMEDIATE, 2);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_IMMEDIATE4, ExecuteOpNOP, IMMEDIATE, 2);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE_X5, ExecuteOpNOP, ZEROPAGE_X, 4);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_IMMEDIATE5, ExecuteOpNOP, IMMEDIATE, 2);
	UOP(NES_OP_DOP_NAME, NES_OP_DOP_ZEROPAGE_X6, ExecuteOpNOP, ZEROPAGE_X, 4);

	// ISC
	UOP(NES_OP_ISC_NAME, NES_OP_ISC_ZEROPAGE, ExecuteOpISC, ZEROPAGE, 5);
	UOP(NES_OP_ISC_NAME, NES_OP_ISC_ZEROPAGE_X, ExecuteOpISC, ZEROPAGE_X, 6);
	UOP(NES_OP_ISC_NAME, NES_OP_ISC_ABSOLUTE, ExecuteOpISC, ABSOLUTE, 6);
	UOP(NES_OP_ISC_NAME, NES_OP_ISC_ABSOLUTE_X, ExecuteOpISC, ABSOLUTE_X, 7);
	UOP(NES_OP_ISC_NAME, NES_OP_ISC_ABSOLUTE_Y, ExecuteOpISC, ABSOLUTE_Y, 7);
	UOP(NES_OP_ISC_NAME, NES_OP_ISC_INDIRECT_X, ExecuteOpISC, INDIRECT_X, 8);
	UOP(NES_OP_ISC_NAME, NES_OP_ISC_INDIRECT_Y, ExecuteOpISC, INDIRECT_Y, 8);

	// KIL
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED1, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED2, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED3, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED4, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED5, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED6, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED7, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED8, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED9, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED10, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED11, ExecuteOpKIL, IMPLIED, 0);
	UOP(NES_OP_KIL_NAME, NES_OP_KIL_IMPLIED12, ExecuteOpKIL, IMPLIED, 0);
			
	// LAR
	UOP(NES_OP_LAS_NAME, NES_OP_LAS_ABSOLUTE_Y, ExecuteOpLAS, ABSOLUTE_Y, 4);

	// LAX
	UOP(NES_OP_LAX_NAME, NES_OP_LAX_ZEROPAGE, ExecuteOpLAX, ZEROPAGE, 3);
	UOP(NES_OP_LAX_NAME, NES_OP_LAX_ZEROPAGE_Y, ExecuteOpLAX, ZEROPAGE_Y, 4);
	UOP(NES_OP_LAX_NAME, NES_OP_LAX_ABSOLUTE, ExecuteOpLAX, ABSOLUTE, 4);
	UOP(NES_OP_LAX_NAME, NES_OP_LAX_ABSOLUTE_Y, ExecuteOpLAX, ABSOLUTE_Y, 4);
	UOP(NES_OP_LAX_NAME, NES_OP_LAX_INDIRECT_X, ExecuteOpLAX, INDIRECT_X, 6);
	UOP(NES_OP_LAX_NAME, NES_OP_LAX_INDIRECT_Y, ExecuteOpLAX, INDIRECT_Y, 5);
	UOP(NES_OP_LAX_NAME, NES_OP_LAX_IMMEDIATE, ExecuteOpLAX, IMMEDIATE, 2);

	// NOP (Unofficial)
	UOP(NES_OP_NOP_NAME, NES_OP_NOP_U_IMPLIED1, ExecuteOpNOP, IMPLIED, 2);
	UOP(NES_OP_NOP_NAME, NES_OP_NOP_U_IMPLIED2, ExecuteOpNOP, IMPLIED, 2);
	UOP(NES_OP_NOP_NAME, NES_OP_NOP_U_IMPLIED3, ExecuteOpNOP, IMPLIED, 2);
	UOP(NES_OP_NOP_NAME, NES_OP_NOP_U_IMPLIED4, ExecuteOpNOP, IMPLIED, 2);
	UOP(NES_OP_NOP_NAME, NES_OP_NOP_U_IMPLIED5, ExecuteOpNOP, IMPLIED, 2);
	UOP(NES_OP_NOP_NAME, NES_OP_NOP_U_IMPLIED6, ExecuteOpNOP, IMPLIED, 2);

	// RLA
	UOP(NES_OP_RLA_NAME, NES_OP_RLA_ZEROPAGE, ExecuteOpRLA, ZEROPAGE, 5);
	UOP(NES_OP_RLA_NAME, NES_OP_RLA_ZEROPAGE_X, ExecuteOpRLA, ZEROPAGE_X, 6);
	UOP(NES_OP_RLA_NAME, NES_OP_RLA_ABSOLUTE, ExecuteOpRLA, ABSOLUTE, 6);
	UOP(NES_OP_RLA_NAME, NES_OP_RLA_ABSOLUTE_X, ExecuteOpRLA, ABSOLUTE_X, 7);
	UOP(NES_OP_RLA_NAME, NES_OP_RLA_ABSOLUTE_Y, ExecuteOpRLA, ABSOLUTE_Y, 7);
	UOP(NES_OP_RLA_NAME, NES_OP_RLA_INDIRECT_X, ExecuteOpRLA, INDIRECT_X, 8);
	UOP(NES_OP_RLA_NAME, NES_OP_RLA_INDIRECT_Y, ExecuteOpRLA, INDIRECT_Y, 8);

	// RRA
	UOP(NES_OP_RRA_NAME, NES_OP_RRA_ZEROPAGE, ExecuteOpRRA, ZEROPAGE, 5);
	UOP(NES_OP_RRA_NAME, NES_OP_RRA_ZEROPAGE_X, ExecuteOpRRA, ZEROPAGE_X, 6);
	UOP(NES_OP_RRA_NAME, NES_OP_RRA_ABSOLUTE, ExecuteOpRRA, ABSOLUTE, 6);
	UOP(NES_OP_RRA_NAME, NES_OP_RRA_ABSOLUTE_X, ExecuteOpRRA, ABSOLUTE_X, 7);
	UOP(NES_OP_RRA_NAME, NES_OP_RRA_ABSOLUTE_Y, ExecuteOpRRA, ABSOLUTE_Y, 7);
	UOP(NES_OP_RRA_NAME, NES_OP_RRA_INDIRECT_X, ExecuteOpRRA, INDIRECT_X, 8);
	UOP(NES_OP_RRA_NAME, NES_OP_RRA_INDIRECT_Y, ExecuteOpRRA, INDIRECT_Y, 8);

	// SBC (Unofficial)
	UOP(NES_OP_SBC_NAME, NES_OP_SBC_U_IMMEDIATE, ExecuteOpSBC, IMMEDIATE, 2);

	// SLO
	UOP(NES_OP_SLO_NAME, NES_OP_SLO_ZEROPAGE, ExecuteOpSLO, ZEROPAGE, 5);
	UOP(NES_OP_SLO_NAME, NES_OP_SLO_ZEROPAGE_X, ExecuteOpSLO, ZEROPAGE_X, 6);
	UOP(NES_OP_SLO_NAME, NES_OP_SLO_ABSOLUTE, ExecuteOpSLO, ABSOLUTE, 6);
	UOP(NES_OP_SLO_NAME, NES_OP_SLO_ABSOLUTE_X, ExecuteOpSLO, ABSOLUTE_X, 7);
	UOP(NES_OP_SLO_NAME, NES_OP_SLO_ABSOLUTE_Y, ExecuteOpSLO, ABSOLUTE_Y, 7);
	UOP(NES_OP_SLO_NAME, NES_OP_SLO_INDIRECT_X, ExecuteOpSLO, INDIRECT_X, 8);
	UOP(NES_OP_SLO_NAME, NES_OP_SLO_INDIRECT_Y, ExecuteOpSLO, INDIRECT_Y, 8);

	// SRE
	UOP(NES_OP_SRE_NAME, NES_OP_SRE_ZEROPAGE, ExecuteOpSRE, ZEROPAGE, 5);
	UOP(NES_OP_SRE_NAME, NES_OP_SRE_ZEROPAGE_X, ExecuteOpSRE, ZEROPAGE_X, 6);
	UOP(NES_OP_SRE_NAME, NES_OP_SRE_ABSOLUTE, ExecuteOpSRE, ABSOLUTE, 6);
	UOP(NES_OP_SRE_NAME, NES_OP_SRE_ABSOLUTE_X, ExecuteOpSRE, ABSOLUTE_X, 7);
	UOP(NES_OP_SRE_NAME, NES_OP_SRE_ABSOLUTE_Y, ExecuteOpSRE, ABSOLUTE_Y, 7);
	UOP(NES_OP_SRE_NAME, NES_OP_SRE_INDIRECT_X, ExecuteOpSRE, INDIRECT_X, 8);
	UOP(NES_OP_SRE_NAME, NES_OP_SRE_INDIRECT_Y, ExecuteOpSRE, INDIRECT_Y, 8);

	// SHX
	UOP(NES_OP_SHX_NAME, NES_OP_SHX_ABSOLUTE_Y, ExecuteOpSHX, ABSOLUTE_Y, 5);

	// SHY
	UOP(NES_OP_SHY_NAME, NES_OP_SHY_ABSOLUTE_X, ExecuteOpSHY, ABSOLUTE_X, 5);

	// TOP (Triple NOP)
	UOP(NES_OP_TOP_NAME, NES_OP_TOP_ABSOLUTE, ExecuteOpNOP, ABSOLUTE, 4);
	UOP(NES_OP_TOP_NAME, NES_OP_TOP_ABSOLUTE_X1, ExecuteOpNOP, ABSOLUTE_X, 4);
	UOP(NES_OP_TOP_NAME, NES_OP_TOP_ABSOLUTE_X2, ExecuteOpNOP, ABSOLUTE_X, 4);
	UOP(NES_OP_TOP_NAME, NES_OP_TOP_ABSOLUTE_X3, ExecuteOpNOP, ABSOLUTE_X, 4);
	UOP(NES_OP_TOP_NAME, NES_OP_TOP_ABSOLUTE_X4, ExecuteOpNOP, ABSOLUTE_X, 4);
	UOP(NES_OP_TOP_NAME, NES_OP_TOP_ABSOLUTE_X5, ExecuteOpNOP, ABSOLUTE_X, 4);
	UOP(NES_OP_TOP_NAME, NES_OP_TOP_ABSOLUTE_X6, ExecuteOpNOP, ABSOLUTE_X, 4);

	// TAS
	UOP(NES_OP_TAS_NAME, NES_OP_TAS_ABSOLUTE_Y, ExecuteOpTAS, ABSOLUTE_Y, 5);

	// XAA
	UOP(NES_OP_XAA_NAME, NES_OP_XAA_IMMEDIATE, ExecuteOpXAA, IMMEDIATE, 2);
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
	const auto it = opInfos_.find(op);
	if (it == opInfos_.end())
		return NESCPUOpAddrMode::UNKNOWN;

	return it->second.addrMode;
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

	// Locate the mapping for this opcode.
	const auto it = opInfos_.find(currentOp_.op);
	if (it == opInfos_.end())
	{
		std::ostringstream oss;
		oss << "Unknown opcode: 0x" << std::hex << +currentOp_.op;
		throw NESCPUExecutionException(oss.str(), reg_);
	}

	auto argInfo = ReadOpArgInfo(it->second.addrMode);
	currentOp_.opCycleCount = it->second.cycleCount;
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
	// @TODO: Possibly inline all instructions instead of using func pointers?
	//        For now, just make all instructions inlined if I decide to change
	//        this in the future...
	(this->*it->second.opFunc)(argInfo);

	// Go to the next instruction if the CPU isn't now jammed...
	if (!currentOp_.opChangedPC && !isJammed_)
		reg_.PC += GetOpSizeFromAddrMode(it->second.addrMode);
}