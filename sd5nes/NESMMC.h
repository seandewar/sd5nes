#pragma once

#include <array>
#include <vector>

#include "NESPPU.h"

/**
* The type of MMC.
*/
enum class NESMMCType
{
	NROM,
	MMC1,
	UNKNOWN
};

/**
* Base class for MMCs.
*/
class INESMMC : public INESMemoryInterface
{
public:
	INESMMC() { }
	virtual ~INESMMC() { }

	virtual NESMMCType GetType() const = 0;
};

/**
* NROM (No mapper).
*/
class NESMMCNROM : public INESMMC
{
public:
	NESMMCNROM(NESMemSRAMBank& sram,
		NESMemCHRBank& chr,
		const NESMemPRGROMBank& prg1,
		const NESMemPRGROMBank* prg2 = nullptr);
	virtual ~NESMMCNROM();

	inline NESMMCType GetType() const override { return NESMMCType::NROM; }

	void Write8(u16 addr, u8 val) override;
	u8 Read8(u16 addr) const override;

private:
	NESMemSRAMBank& sram_;
	NESMemCHRBank& chr_;
	std::array<const NESMemPRGROMBank*, 2> prg_;
};

/**
* Nintendo MMC1.
*/
class NESMMC1 : public INESMMC
{
public:
	NESMMC1(const std::vector<NESMemSRAMBank>& sram,
		const std::vector<NESMemCHRBank>& chr,
		const std::vector<NESMemPRGROMBank>& prg,
		NESNameTableMirroringType& ntMirror);
	virtual ~NESMMC1();

	inline NESMMCType GetType() const override { return NESMMCType::MMC1; }

	void Write8(u16 addr, u8 val) override;
	u8 Read8(u16 addr) const override;

private:
	std::vector<NESMemSRAMBank> sram_;
	std::vector<NESMemCHRBank> chr_;
	const std::vector<NESMemPRGROMBank> prg_;
	NESNameTableMirroringType& ntMirror_;

	std::array<std::size_t, 2> chrBankIndices_;
	std::array<std::size_t, 2> prgBankIndices_;

	// shift reg, chr mode, prg mode [5-bits].
	u8 shiftReg_;
	u8 prgBankMode_, chrBankMode_;

	void WriteControlRegister(u8 val);
	void WriteCHRBankRegister(u8 bankNum, u8 val);
	void WritePRGBankRegister(u8 val);

	void HandleRegisterWrite(u16 addr, u8 val);
};