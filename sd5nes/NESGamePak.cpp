#include "NESGamePak.h"

#include <fstream>
#include <sstream>

#include "NESReadBuffer.h"
#include "NESMMC.h"


NESGamePak::NESGamePak()
{
	ResetLoadedState();
}


NESGamePak::~NESGamePak()
{
}

void NESGamePak::ResetLoadedState()
{
	isRomLoaded_ = false;

	mapperType_ = NESMMCType::UNKNOWN;
	mirrorType_ = NESNameTableMirroringType::UNKNOWN;
	hasBatteryPackedRam_ = hasTrainer_ = false;

	romFileName_.clear();

	prgBanks_.clear();
	chrBanks_.clear();
	sramBanks_.clear();
}


std::vector<u8> NESGamePak::ReadROMFile(const std::string& fileName)
{
	std::ifstream fileStream(fileName, std::ios_base::in | std::ios_base::binary);
	std::vector<u8> fileData;

	// Read ROM file.
	while (fileStream.good())
		fileData.emplace_back(fileStream.get());

	// Check if end-of-file (successful read).
	if (fileStream.eof())
		return fileData;

	// If not EOF, then read failed.
	throw NESGamePakLoadException("Failed to read NES GamePak ROM image!");
}


std::unique_ptr<NESGamePakPowerState> NESGamePak::GetNewGamePakPowerState() const
{
	return std::make_unique<NESGamePakPowerState>(mapperType_,
												  prgBanks_,
												  chrBanks_,
												  sramBanks_,
												  mirrorType_,
												  hasBatteryPackedRam_);
}


/* Holds the index number for the different ROM infos stored in the iNES format */
#define INES_PRGROM_BANKS_INDEX 0
#define INES_CHRROM_BANKS_INDEX 1
#define INES_ROM_CONTROL_1_INDEX 2
#define INES_ROM_CONTROL_2_INDEX 3
#define INES_RAM_BANKS_INDEX 4


void NESGamePak::ParseROMFileData(const std::vector<u8>& data)
{
	// @TODO Whole function might need some optimizations if too much
	// stuff is being copied all around the place...?
	NESReadBuffer buf(data);
	std::vector<u8> romInfo;
	try
	{
		// Read file type & support iNES files.
		const auto type = buf.ReadNextStr(4);
		if (type != std::string("NES") + static_cast<char>(0x1A))
			throw NESGamePakLoadException("Unexpected ROM image format!");

		// Read ROM info bytes & skip past the 7 reserved bytes.
		romInfo = buf.ReadNext(5);
		buf.ReadNext(7);

		// Read number of 8KB SRAM banks, and create the specified amount.
		for (std::size_t i = 0; i < romInfo[INES_RAM_BANKS_INDEX]; ++i)
			sramBanks_.emplace_back();

		// Assume 1 bank if this is 0 for compatibility reasons.
		if (sramBanks_.size() == 0)
			sramBanks_.emplace_back();

		// Check what NT mirroring is being used.
		if ((romInfo[INES_ROM_CONTROL_1_INDEX] & 8) == 8)
			mirrorType_ = NESNameTableMirroringType::FOUR_SCREEN;
		else
		{
			if ((romInfo[INES_ROM_CONTROL_1_INDEX] & 1) == 1)
				mirrorType_ = NESNameTableMirroringType::VERTICAL;
			else
				mirrorType_ = NESNameTableMirroringType::HORIZONTAL;
		}

		// Check if the image has a trainer or battery-packed RAM.
		hasBatteryPackedRam_ = ((romInfo[INES_ROM_CONTROL_1_INDEX] & 2) == 2);
		hasTrainer_ = ((romInfo[INES_ROM_CONTROL_1_INDEX] & 4) == 4);
	}
	catch (const NESReadBufferException&)
	{
		throw NESGamePakLoadException("Failed to parse ROM image header!");
	}

	try
	{
		// If there is a trainer, ignore it.
		if (hasTrainer_)
			buf.ReadNext(0x200);

		// Copy contents of PRGROM and CHRROM into memory.
		for (std::size_t i = 0; i < romInfo[INES_PRGROM_BANKS_INDEX]; ++i)
			prgBanks_.emplace_back(buf.ReadNext(0x4000));

		if (prgBanks_.size() == 0)
			throw NESGamePakLoadException("No PRG-ROM in ROM image!");

		for (std::size_t i = 0; i < romInfo[INES_CHRROM_BANKS_INDEX]; ++i)
			chrBanks_.emplace_back(buf.ReadNext(0x2000));

		// If we have no CHR-ROM banks then just create an empty one to represent CHR-RAM.
		if (chrBanks_.size() == 0)
			chrBanks_.emplace_back();
	}
	catch (const NESReadBufferException&)
	{
		throw NESGamePakLoadException("Failed to parse ROM image data!");
	}

	// Get the mapper number and assign an MMC type to it.
	const u8 mapperNumber = (romInfo[INES_ROM_CONTROL_2_INDEX] & 0xF0) | (romInfo[INES_ROM_CONTROL_1_INDEX] >> 4);
	switch (mapperNumber)
	{
	case 0:
		mapperType_ = NESMMCType::NROM;
		break;

	case 1:
		mapperType_ = NESMMCType::MMC1;
		break;

	default:
		std::ostringstream oss;
		oss << "Unsupported ROM image mapper " << +mapperNumber;
		throw NESGamePakLoadException(oss.str());
	}
}


void NESGamePak::LoadROM(const std::string& fileName)
{
	ResetLoadedState();

	romFileName_ = fileName;
	ParseROMFileData(ReadROMFile(fileName));

	isRomLoaded_ = true;
}