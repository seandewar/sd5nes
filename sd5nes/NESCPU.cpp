#include "NESCPU.h"

#include "NESCPUConstants.h"
#include "NESHelper.h"

#include <cassert>


std::map<u8, NESCPUOpInfo> NESCPU::opInfos_;
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


// Defines variables argValVarName and crossedPageBoundaryVarName
// and auto returns false from the host op function if failed read.
// NOTE: It is OK to use a semi-colon after using this macro function.
#define OP_HANDLE_READ_ARG(argValVarName, crossedPageBoundaryVarName) \
		u8 argValVarName; \
		bool crossedPageBoundaryVarName; \
		if (!ReadOpArgValue(&argValVarName, &crossedPageBoundaryVarName)) { \
			return false; \
		} \


// Defines variable argValVarName
// and auto returns false from the host op function if failed read.
// NOTE: It is OK to use a semi-colon after using this macro function.
#define OP_HANDLE_READ_ARG_SIMPLE(argValVarName) \
		u8 argValVarName; \
		if (!ReadOpArgValue(&argValVarName, nullptr)) { \
			return false; \
		} \


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
	case NESCPUOpAddressingMode::IMPLIED:
		opSize = 1;
		break;

	// 2 byte addressing modes.
	case NESCPUOpAddressingMode::IMMEDIATE:
	case NESCPUOpAddressingMode::ZEROPAGE:
	case NESCPUOpAddressingMode::ZEROPAGE_X:
	case NESCPUOpAddressingMode::ZEROPAGE_Y:
	case NESCPUOpAddressingMode::INDIRECT_X:
	case NESCPUOpAddressingMode::INDIRECT_Y:
		opSize = 2;
		break;

	// 3 byte addressing modes.
	case NESCPUOpAddressingMode::ABSOLUTE:
	case NESCPUOpAddressingMode::ABSOLUTE_X:
	case NESCPUOpAddressingMode::ABSOLUTE_Y:
	case NESCPUOpAddressingMode::INDIRECT:
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
	currentOpChangedPC_ = false;
	if (!(this->*it->second.opFunc)())
		return false; // Failed to execute opcode - program execution error.

	int opSize;
	if (!GetOpSizeFromAddressingMode(it->second.addrMode, &opSize))
		return false;

	// Go to the next instruction.
	if (!currentOpChangedPC_)
		reg_.PC += opSize;

	return true;
}


bool NESCPU::ReadOpArgValue(u8* outVal, bool* outCrossedPageBoundary)
{
	u16 addr;
	bool crossedPageBoundary;

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
		crossedPageBoundary = false;
		addr = reg_.PC + 1;
		break;


	case NESCPUOpAddressingMode::ABSOLUTE:
	case NESCPUOpAddressingMode::INDIRECT:
		crossedPageBoundary = false;
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;
		break;


	case NESCPUOpAddressingMode::ABSOLUTE_X:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;

		crossedPageBoundary = !NESHelper::IsInSamePage(addr, addr + reg_.X);
		addr += reg_.X;
		break;


	case NESCPUOpAddressingMode::ABSOLUTE_Y:
		if (!mem_.Read16(reg_.PC + 1, &addr))
			return false;

		crossedPageBoundary = !NESHelper::IsInSamePage(addr, addr + reg_.Y);
		addr += reg_.Y;
		break;


		// 8-bit representation of address used to read Zero Page and Indirect addressing types.
		u8 addr8;


	case NESCPUOpAddressingMode::ZEROPAGE:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		crossedPageBoundary = false;
		addr = addr8;
		break;


	case NESCPUOpAddressingMode::ZEROPAGE_X:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		crossedPageBoundary = false;
		addr8 += reg_.X; // Will wrap around if X is too big.
		addr = addr8;
		break;


	case NESCPUOpAddressingMode::ZEROPAGE_Y:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		crossedPageBoundary = false;
		addr8 += reg_.Y; // Will wrap around if X is too big.
		addr = addr8;
		break;


	case NESCPUOpAddressingMode::INDIRECT_X:
		if (!mem_.Read8(reg_.PC + 1, &addr8))
			return false;

		crossedPageBoundary = false;
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

		crossedPageBoundary = !NESHelper::IsInSamePage(addr, addr + reg_.Y);
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
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

	// ADC takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A + M + C -> A, C
	// NOTE: NES 6502 variant has no BCD mode.
	const uleast16 res = reg_.A + argVal + reg_.C;

	reg_.C = (res > 0xFF ? 1 : 0);
	UpdateRegN(static_cast<u8>(res));
	reg_.V = (((~(reg_.A ^ argVal) & (reg_.A ^ res)) >> 7) & 1); // Check if the sign has changed due to overflow.
	reg_.A = static_cast<u8>(res);
	return true;
}


bool NESCPU::ExecuteOpAND()
{
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

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
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// C <- [76543210] <- 0
	const u8 res = (argVal << 1);
	if (!WriteOpResult(res))
		return false;

	reg_.C = ((argVal >> 7) & 1); // Set carry bit if bit 7 was originally 1.
	UpdateRegZ(res);
	UpdateRegN(res);
	return true;
}


bool NESCPU::ExecuteOpAsBranch(bool shouldBranch, int branchSamePageCycleExtra, int branchDiffPageCycleExtra)
{
	if (!shouldBranch)
		return true;

	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	const u16 jumpPC = reg_.PC + argVal;

	// Check if the branch will cross a page boundary.
	if (!NESHelper::IsInSamePage(reg_.PC, jumpPC))
		currentOpCycleCount_ += branchDiffPageCycleExtra;
	else
		currentOpCycleCount_ += branchSamePageCycleExtra;

	UpdateRegPC(jumpPC);
	return true;
}


bool NESCPU::ExecuteOpBCC()
{
	// Branch on C = 0
	return ExecuteOpAsBranch((reg_.C == 0), 1, 2);
}


bool NESCPU::ExecuteOpBCS()
{
	// Branch on C = 1
	return ExecuteOpAsBranch((reg_.C == 1), 1, 2);
}


bool NESCPU::ExecuteOpBEQ()
{
	// Branch on Z = 1
	return ExecuteOpAsBranch((reg_.Z == 1), 1, 2);
}


bool NESCPU::ExecuteOpBMI()
{
	// Branch on N = 1
	return ExecuteOpAsBranch((reg_.N == 1), 1, 2);
}


bool NESCPU::ExecuteOpBNE()
{
	// Branch on Z = 0
	return ExecuteOpAsBranch((reg_.Z == 0), 1, 2);
}


bool NESCPU::ExecuteOpBPL()
{
	// Branch on N = 0
	return ExecuteOpAsBranch((reg_.N == 0), 1, 2);
}


bool NESCPU::ExecuteOpBVC()
{
	// Branch on V = 0
	return ExecuteOpAsBranch((reg_.V == 0), 1, 2);
}


bool NESCPU::ExecuteOpBVS()
{
	// Branch on V = 1
	return ExecuteOpAsBranch((reg_.V == 1), 1, 2);
}


bool NESCPU::ExecuteOpBIT()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// A /\ M, M7 -> N, M6 -> V
	UpdateRegN(argVal);
	UpdateRegZ((argVal & reg_.A));
	reg_.V = ((argVal >> 6) & 1);
	return true;
}


bool NESCPU::StackPush8(u8 val)
{
	// Check that the stack is not full.
	if (reg_.SP > NES_CPU_STACK_START + NES_CPU_STACK_SIZE)
		return false;

	if (!mem_.Write8(reg_.SP++, val))
		return false;

	return true;
}


bool NESCPU::StackPush16(u16 val)
{
	// Push most-significant byte first.
	if (!StackPush8((val & 0xFF00) >> 8))
		return false;

	if (!StackPush8((val & 0x00FF)))
		return false;

	return true;
}


bool NESCPU::StackPull8(u8* outVal)
{
	// Check that the stack is not empty.
	if (reg_.SP <= NES_CPU_STACK_START)
		return false;

	// Will write to outVal if it's not null.
	if (!mem_.Read8(--reg_.SP, outVal))
		return false;

	return true;
}


bool NESCPU::StackPull16(u16* outVal)
{
	u8 low, hi;

	// Pull least-significant byte first.
	if (!StackPull8(&low))
		return false;

	if (!StackPull8(&hi))
		return false;

	// Convert to 16-bit val.
	if (outVal != nullptr)
		*outVal = ((hi << 8) | low);

	return true;
}


bool NESCPU::ExecuteInterrupt(NESCPUInterrupt interruptType)
{
	// TODO handle other interrupts.
	switch (interruptType)
	{
	case NESCPUInterrupt::IRQBRK:
		reg_.I = 1;

		u16 val;
		if (!mem_.Read16(NES_CPU_IRQBRK_VECTOR_START, &val))
			return false;

		UpdateRegPC(val);
		break;

	default:
		return false;
	}

	return true;
}


bool NESCPU::ExecuteOpBRK()
{
	// Forced Interrupt PC + 2 toS P toS
	if (!StackPush16(reg_.PC + 1))
		return false;

	reg_.B = 1; // Set break flag before pushing P.
	if (!StackPush16(reg_.P))
		return false;

	return ExecuteInterrupt(NESCPUInterrupt::IRQBRK);
}


bool NESCPU::ExecuteOpCLC()
{
	// 0 -> C
	reg_.C = 0;
	return true;
}


bool NESCPU::ExecuteOpCLD()
{
	// 0 -> D
	reg_.D = 0;
	return true;
}


bool NESCPU::ExecuteOpCLI()
{
	// 0 -> I
	reg_.I = 0;
	return true;
}


bool NESCPU::ExecuteOpCLV()
{
	// 0 -> V
	reg_.V = 0;
	return true;
}


bool NESCPU::ExecuteOpCMP()
{
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

	// CMP takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A - M
	const uleast16 res = reg_.A - argVal;

	reg_.C = (res < 0x100 ? 1 : 0);
	UpdateRegN(static_cast<u8>(res));
	UpdateRegZ((res & 0xFF)); // Check first 8-bits.
	return true;
}


bool NESCPU::ExecuteOpCPX()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// X - M
	const uleast16 res = reg_.X - argVal;

	reg_.C = (res < 0x100 ? 1 : 0);
	UpdateRegN(static_cast<u8>(res));
	UpdateRegZ((res & 0xFF)); // Check first 8-bits.
	return true;
}


bool NESCPU::ExecuteOpCPY()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// Y - M
	const uleast16 res = reg_.Y - argVal;

	reg_.C = (res < 0x100 ? 1 : 0);
	UpdateRegN(static_cast<u8>(res));
	UpdateRegZ((res & 0xFF)); // Check first 8-bits.
	return true;
}


bool NESCPU::ExecuteOpDEC()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// M - 1 -> M
	const u8 res = argVal - 1;
	if (!WriteOpResult(res))
		return false;

	UpdateRegN(res);
	UpdateRegZ(res);
	return true;
}


bool NESCPU::ExecuteOpDEX()
{
	// X - 1 -> X
	--reg_.X;
	UpdateRegN(reg_.X);
	UpdateRegZ(reg_.X);
	return true;
}


bool NESCPU::ExecuteOpDEY()
{
	// Y - 1 -> Y
	--reg_.Y;
	UpdateRegN(reg_.Y);
	UpdateRegZ(reg_.Y);
	return true;
}


bool NESCPU::ExecuteOpEOR()
{
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

	// EOR takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A EOR M -> A
	const u8 res = (reg_.A ^ argVal);

	UpdateRegN(res);
	UpdateRegZ(res);
	reg_.A = res;
	return true;
}


bool NESCPU::ExecuteOpINC()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);
	
	// M + 1 -> M
	const u8 res = argVal + 1;
	if (!WriteOpResult(res))
		return false;

	UpdateRegN(res);
	UpdateRegZ(res);
	return true;
}


bool NESCPU::ExecuteOpINX()
{
	// X + 1 -> X
	++reg_.X;
	UpdateRegN(reg_.X);
	UpdateRegZ(reg_.X);
	return true;
}


bool NESCPU::ExecuteOpINY()
{
	// Y + 1 -> Y
	++reg_.Y;
	UpdateRegN(reg_.Y);
	UpdateRegZ(reg_.Y);
	return true;
}


bool NESCPU::ExecuteOpJMP()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// (PC + 1) -> PCL
	// (PC + 2) -> PCH
	UpdateRegPC(argVal);
	return true;
}


bool NESCPU::ExecuteOpJSR()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// PC + 2 toS, (PC + 1) -> PCL
	//             (PC + 2) -> PCH
	if (!StackPush16(reg_.PC - 1))
		return false;

	UpdateRegPC(argVal);
	return true;
}


bool NESCPU::ExecuteOpLDA()
{
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

	// LDA takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// M -> A
	UpdateRegN(argVal);
	UpdateRegZ(argVal);
	reg_.A = argVal;
	return true;
}


bool NESCPU::ExecuteOpLDX()
{
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

	// LDX takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// M -> X
	UpdateRegN(argVal);
	UpdateRegZ(argVal);
	reg_.X = argVal;
	return true;
}


bool NESCPU::ExecuteOpLDY()
{
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

	// LDY takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// M -> Y
	UpdateRegN(argVal);
	UpdateRegZ(argVal);
	reg_.Y = argVal;
	return true;
}


bool NESCPU::ExecuteOpLSR()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// 0 -> [76543210] -> C
	const u8 res = (argVal >> 1);
	if (!WriteOpResult(res))
		return false;

	reg_.C = ((argVal & 1)); // Set carry bit if bit 0 was originally 1.
	UpdateRegZ(res);
	UpdateRegN(res);
	return true;
}


bool NESCPU::ExecuteOpNOP()
{
	// Do nothing.
	return true;
}


bool NESCPU::ExecuteOpORA()
{
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

	// ORA takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A OR M -> A
	const u8 res = (argVal | reg_.A);

	UpdateRegZ(res);
	UpdateRegN(res);
	reg_.A = res;
	return true;
}


bool NESCPU::ExecuteOpPHA()
{
	// A toS
	return StackPush8(reg_.A);
}


bool NESCPU::ExecuteOpPHP()
{
	// P toS
	return StackPush8(reg_.P);
}


bool NESCPU::ExecuteOpPLA()
{
	// A fromS.
	u8 val;
	if (!StackPull8(&val))
		return false;

	UpdateRegZ(val);
	UpdateRegN(val);
	reg_.A = val;
	return true;
}


bool NESCPU::ExecuteOpPLP()
{
	// P fromS.
	u8 val;
	if (!StackPull8(&val))
		return false;

	UpdateRegZ(val);
	UpdateRegN(val);
	reg_.P = val;
	return true;
}


bool NESCPU::ExecuteOpROL()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// C <- [7654321] <- C
	uleast16 res = (argVal << 1);
	if (!WriteOpResult(static_cast<u8>(res)))
		return false;

	if (reg_.C == 1)
		res |= 1;

	reg_.C = (res > 0xFF ? 1 : 0);
	UpdateRegZ(static_cast<u8>(res));
	UpdateRegN(static_cast<u8>(res));
	return true;
}


bool NESCPU::ExecuteOpROR()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// C -> [7654321] -> C
	uleast16 res = argVal;
	if (!WriteOpResult(static_cast<u8>(res)))
		return false;

	if (reg_.C == 1)
		res |= 0x100;

	reg_.C = (res & 1);
	res >>= 1;
	UpdateRegZ(static_cast<u8>(res));
	UpdateRegN(static_cast<u8>(res));
	return true;
}


bool NESCPU::ExecuteOpRTI()
{
	// P fromS PC fromS
	u8 valP;
	if (!StackPull8(&valP))
		return false;

	u16 newAddr;
	if (!StackPull16(&newAddr))
		return false;

	reg_.P = valP;
	UpdateRegPC(newAddr);
	return true;
}


bool NESCPU::ExecuteOpRTS()
{
	// PC fromS, PC + 1 -> PC
	u16 newAddr;
	if (!StackPull16(&newAddr))
		return false;

	UpdateRegPC(newAddr + 1);
	return true;
}


bool NESCPU::ExecuteOpSBC()
{
	OP_HANDLE_READ_ARG(argVal, crossedPageBoundary);

	// SBC takes 1 extra CPU cycle if a page boundary was crossed.
	if (crossedPageBoundary)
		++currentOpCycleCount_;

	// A - M - C -> A
	const uleast16 res = reg_.A - argVal - (1 - reg_.C);

	reg_.C = (res < 0x100 ? 1 : 0);
	UpdateRegN(static_cast<u8>(res));
	UpdateRegZ(static_cast<u8>(res));
	reg_.V = (((~(reg_.A ^ argVal) & (reg_.A ^ res)) >> 7) & 1); // Check if the sign has changed due to overflow.
	reg_.A = static_cast<u8>(res);
	return true;
}


bool NESCPU::ExecuteOpSEC()
{
	// 1 -> C
	reg_.C = 1;
	return true;
}


bool NESCPU::ExecuteOpSED()
{
	// 1 -> D
	reg_.D = 1;
	return true;
}


bool NESCPU::ExecuteOpSEI()
{
	// 1 -> I
	reg_.I = 1;
	return true;
}


bool NESCPU::ExecuteOpSTA()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// A -> M
	return mem_.Write8(argVal, reg_.A);
}


bool NESCPU::ExecuteOpSTX()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// X -> M
	return mem_.Write8(argVal, reg_.X);
}


bool NESCPU::ExecuteOpSTY()
{
	OP_HANDLE_READ_ARG_SIMPLE(argVal);

	// Y -> M
	return mem_.Write8(argVal, reg_.Y);
}


bool NESCPU::ExecuteOpTAX()
{
	// A -> X
	reg_.X = reg_.A;
	return true;
}


bool NESCPU::ExecuteOpTAY()
{
	// A -> Y
	reg_.Y = reg_.A;
	return true;
}


bool NESCPU::ExecuteOpTSX()
{
	// S -> X
	reg_.X = reg_.SP;
	return true;
}


bool NESCPU::ExecuteOpTXA()
{
	// X -> A
	reg_.A = reg_.X;
	return true;
}


bool NESCPU::ExecuteOpTXS()
{
	// X -> S
	reg_.SP = reg_.X;
	return true;
}


bool NESCPU::ExecuteOpTYA()
{
	// Y -> A
	reg_.A = reg_.Y;
	return true;
}