#pragma once

#include <memory>

#include "NESMMC.h"

/**
* Represents the internal active state of a GamePak when it is powered on
* and to be used by the NES.
*/
class NESGamePakPowerState
{
public:
	NESGamePakPowerState(NESMMCType mapperType,
		const std::vector<NESMemPRGROMBank>& prg,
		const std::vector<NESMemCHRBank>& chr,
		const std::vector<NESMemSRAMBank>& sram,
		NESNameTableMirroringType mirrorType,
		bool hasBatteryPackedRam);
	~NESGamePakPowerState();

	inline NESMMCType GetMMCType() const { return mapperType_; }
	inline INESMMC& GetMMC() const { return *mmc_; }

	inline const std::vector<NESMemPRGROMBank>& GetPRGBanksRef() { return prgBanks_; }
	inline const std::vector<NESMemCHRBank>& GetCHRBanksRef() { return chrBanks_; }
	inline const std::vector<NESMemSRAMBank>& GetSRAMBanksRef() { return sramBanks_; }

	inline NESNameTableMirroringType& GetNameTableMirroringRef() { return mirrorType_; }

	inline bool HasBatteryPackedRAM() const { return hasBatteryPackedRam_; }

private:
	const NESMMCType mapperType_;
	NESNameTableMirroringType mirrorType_;

	/* The active MMC, SRAM, PRG-ROM and CHR-ROM of the cart. */
	std::unique_ptr<INESMMC> mmc_;
	const std::vector<NESMemPRGROMBank> prgBanks_;
	std::vector<NESMemCHRBank> chrBanks_;
	std::vector<NESMemSRAMBank> sramBanks_;

	const bool hasBatteryPackedRam_;

	void CreateMapper();
};

