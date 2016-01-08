#pragma once

#include <array>

#include "NESGamePak.h"

/**
* The type of MMC.
*/
enum class NESMMCType
{
	NROM, // NROM - No mapper.
	UNKNOWN // Unknown mapper.
};

/**
* Base class for MMCs.
*/
class NESMMC : public INESMemoryInterface
{
public:
	explicit NESMMC(NESMemSRAM& sram);
	virtual ~NESMMC();

	/**
	* Gets the type of MMC used.
	*/
	virtual NESMMCType GetType() const = 0;

	/**
	* Write 8-bits to MMC mapped memory.
	*/
	virtual void Write8(u16 addr, u8 val);

	/**
	* Reads 8-bits from MMC mapped memory.
	*/
	virtual u8 Read8(u16 addr) const;

protected:
	NESMemSRAM& sram_;
};

/**
* NROM MMC.
*/
class NESMMCNROM : public NESMMC
{
public:
	NESMMCNROM(NESMemSRAM& sram, NESMemCHRROMBank* chr, NESMemPRGROMBank& prg1, NESMemPRGROMBank* prg2 = nullptr);
	virtual ~NESMMCNROM();

	inline NESMMCType GetType() const override { return NESMMCType::NROM; }

	void Write8(u16 addr, u8 val) override;
	u8 Read8(u16 addr) const override;

private:
	NESMemCHRROMBank* chr_;
	std::array<NESMemPRGROMBank*, 2> prg_;
};