#include "NESGamePakPowerState.h"


NESGamePakPowerState::NESGamePakPowerState(NESMMCType mapperType,
										   const std::vector<NESMemPRGROMBank>& prg,
										   const std::vector<NESMemCHRBank>& chr,
										   const std::vector<NESMemSRAMBank>& sram,
										   NESNameTableMirroringType mirrorType,
										   bool hasBatteryPackedRam) :
mapperType_(mapperType),
prgBanks_(prg),
chrBanks_(chr),
sramBanks_(sram),
mirrorType_(mirrorType),
hasBatteryPackedRam_(hasBatteryPackedRam)
{
	CreateMapper();
}


NESGamePakPowerState::~NESGamePakPowerState()
{
}


void NESGamePakPowerState::CreateMapper()
{
	assert(mapperType_ != NESMMCType::UNKNOWN);

	switch (mapperType_)
	{
	case NESMMCType::NROM:
		mmc_ = std::make_unique<NESMMCNROM>(
			sramBanks_[0],
			chrBanks_[0],
			prgBanks_[0],
			(prgBanks_.size() > 1 ? &prgBanks_[1] : nullptr)
		);
		break;

	case NESMMCType::MMC1:
		mmc_ = std::make_unique<NESMMC1>(sramBanks_, chrBanks_, prgBanks_, mirrorType_);
		break;
	}

	assert(mmc_ != nullptr);
}