#pragma once

#include <vector>
#include <functional>

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
class NESMMC : public NESMemoryMapper
{
	friend class NESCPUMemoryMapper; // Allows the NESCPUMemoryMapper to access GetMapping()

public:
	NESMMC();
	virtual ~NESMMC();

	/**
	* Gets the type of MMC used.
	*/
	virtual NESMMCType GetType() const = 0;

protected:
	std::vector<std::reference_wrapper<NESMemPRGROMBank>> loadedPrgRomBanks_;

	inline virtual std::pair<INESMemoryInterface*, u16> GetMapping(u16 addr) const override 
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
	NESMMCNROM(const std::array<std::reference_wrapper<NESMemPRGROMBank>, 2>& prgRomBanks);
	virtual ~NESMMCNROM();

	inline NESMMCType GetType() const override { return NESMMCType::NROM; }

protected:
	std::pair<INESMemoryInterface*, u16> GetMapping(u16 addr) const override;
};