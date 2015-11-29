#pragma once

#include <array>

#include "NESMemory.h"

/**
* The type of MMC.
*/
enum class NESMMCType
{
	NROM, // NROM - No mapper.
	UNKNOWN // Unknown mapper.
};

typedef NESMemory<0x4000> NESMemPRGROMBank;
typedef NESMemory<0x2000> NESMemCHRROMBank;

/**
* Base class for creating MMCs for allowing bank switching .etc
*/
class NESMMC : public INESMemoryInterface
{
public:
	NESMMC();
	virtual ~NESMMC();

	/**
	* Write 8-bits to MMC mapped memory.
	* Assumes all MMC memory is read-only.
	*/
	inline virtual void Write8(u16 addr, u8 val) { /* Do nothing - assume memory is read-only. */ }

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

	/**
	* Gets the MMC mapping of an address.
	*/
	inline virtual std::pair<const INESMemoryInterface*, u16> GetMMCMapping(u16 addr) const
	{ 
		// Default behaviour - return supplied address.
		return std::make_pair(nullptr, addr);
	}
};

/**
* NROM MMC.
*/
class NESMMCNROM : public NESMMC
{
public:
	NESMMCNROM(NESMemPRGROMBank& prgRomBank);
	NESMMCNROM(NESMemPRGROMBank& prgRom1, NESMemPRGROMBank& prgRom2);
	virtual ~NESMMCNROM();

	inline NESMMCType GetType() const override { return NESMMCType::NROM; }

protected:
	std::pair<const INESMemoryInterface*, u16> GetMMCMapping(u16 addr) const override;
};