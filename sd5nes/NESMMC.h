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
	* Write 8-bits to MMC mapped memory.
	*/
	virtual void Write8(u16 addr, u8 val);

	/**
	* Reads 8-bits from MMC mapped memory.
	*/
	virtual u8 Read8(u16 addr) const;

	/**
	* Gets the type of MMC used.
	*/
	virtual NESMMCType GetType() const = 0;

protected:
	std::array<const NESMemPRGROMBank*, 2> loadedPrgRomBanks_;
	NESMemSRAM& sram_;
};

/**
* NROM MMC.
*/
class NESMMCNROM : public NESMMC
{
public:
	NESMMCNROM(NESMemSRAM& sram, NESMemPRGROMBank& prgRomBank);
	NESMMCNROM(NESMemSRAM& sram, NESMemPRGROMBank& prgRom1, NESMemPRGROMBank& prgRom2);
	virtual ~NESMMCNROM();

	inline NESMMCType GetType() const override { return NESMMCType::NROM; }
};