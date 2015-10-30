#include "NESGamePak.h"

#include <fstream>

#include "NESReadBuffer.h"


NESGamePak::NESGamePak() :
isRomLoaded_(false),
hasBatteryPackedRam_(false),
hasTrainer_(false),
mirrorType_(NESMirroringType::UNKNOWN),
mapperType_(NESMapperType::UNKNOWN)
{
}


NESGamePak::~NESGamePak()
{
}


bool NESGamePak::ReadROMFile(const std::string& fileName)
{
	std::ifstream fileStream(fileName, std::ios_base::in | std::ios_base::binary);
	std::vector<u8> fileData;

	// Read ROM file.
	while (fileStream.good())
		fileData.emplace_back(fileStream.get());

	// Check if end-of-file.
	if (fileStream.eof())
	{
		// Read succeeded.
		romFileName_ = fileName;
		romFileData_ = fileData;
		return true;
	}

	// If not EOF, then read failed.
	return false;
}


/* Holds the index number for the different ROM infos stored in the iNES format */
#define INES_PRGROM_BANKS_INDEX 0
#define INES_CHRROM_BANKS_INDEX 1
#define INES_ROM_CONTROL_1_INDEX 2
#define INES_ROM_CONTROL_2_INDEX 3
#define INES_RAM_BANKS_INDEX 4


bool NESGamePak::ParseROMFileData()
{
	if (!isRomLoaded_)
		return false;

	NESReadBuffer buf(romFileData_);

	// Read file type.
	std::string type;
	if (!buf.ReadNextStr(4, &type))
		return false;

	// Only support iNES files for now...
	// @TODO Support others?
	if (type != std::string("NES") + (char)0x1A)
		return false;
	
	// Vector containing the different ROM info bytes.
	// Read ROM info bytes & skip past the 7 reserved bytes.
	std::vector<u8> romInfo;
	if (!buf.ReadNext(5, &romInfo) || !buf.ReadNext(7, nullptr))
		return false;

	// @TODO Assume 1 page of RAM if # 8KB RAM banks is 0.

	// Check if bit 3 is 1 = four screen mirroring.
	// If bit 3 is 0, check bit 0. If bit 0 is 1 = vertical. 0 = horizontal.
	if ((romInfo[INES_ROM_CONTROL_1_INDEX] & 8) == 8)
		mirrorType_ = NESMirroringType::FOUR_SCREEN;
	else
		mirrorType_ = ((romInfo[INES_ROM_CONTROL_1_INDEX] & 1) == 1 ? NESMirroringType::VERTICAL : NESMirroringType::HORIZONTAL);

	// Check bits 1 and 2 = battery packed RAM & trainer respectively.
	hasBatteryPackedRam_ = ((romInfo[INES_ROM_CONTROL_1_INDEX] & 2) == 2);
	hasTrainer_ = ((romInfo[INES_ROM_CONTROL_1_INDEX] & 4) == 4);

	// Get the mapper number using bits 4-7 from ROM Control Byte 1 and 2.
	// Convert mapper num to enum.
	const u8 mapperNum = ((romInfo[INES_ROM_CONTROL_2_INDEX] & 0xF0) | (romInfo[INES_ROM_CONTROL_1_INDEX] >> 4));
	switch (mapperNum)
	{
	case 0:
		mapperType_ = NESMapperType::NROM;
		break;

		// Unknown mapper type!
	default:
		mapperType_ = NESMapperType::UNKNOWN;
		return false;
	}

	return true;
}


bool NESGamePak::LoadROM(const std::string& fileName)
{
	// Reset current loaded flag.
	isRomLoaded_ = false;

	if (!ReadROMFile(fileName))
		return false;

	if (!ParseROMFileData())
		return false;

	isRomLoaded_ = true;
	return true;
}


bool NESGamePak::IsROMLoaded() const
{
	return isRomLoaded_;
}


const NESMemory& NESGamePak::GetProgramROM() const
{
	return prgRom_;
}


const NESMemory& NESGamePak::GetCharacterROM() const
{
	return chrRom_;
}


NESMirroringType NESGamePak::GetMirroringType() const
{
	return mirrorType_;
}


NESMapperType NESGamePak::GetMapperType() const
{
	return mapperType_;
}


bool NESGamePak::HasBatteryPackedRAM() const
{
	return hasBatteryPackedRam_;
}


bool NESGamePak::HasTrainer() const
{
	return hasTrainer_;
}