#include "NESGamePak.h"

#include <fstream>

#include "NESReadBuffer.h"


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

	mirrorType_ = NESMirroringType::UNKNOWN;
	hasBatteryPackedRam_ = false;
	hasTrainer_ = false;
	ramBanks_ = 0;

	mmc_.reset();
	romFileName_.clear();
	prgRomBanks_.clear();
	chrRomBanks_.clear();
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


/* Holds the index number for the different ROM infos stored in the iNES format */
#define INES_PRGROM_BANKS_INDEX 0
#define INES_CHRROM_BANKS_INDEX 1
#define INES_ROM_CONTROL_1_INDEX 2
#define INES_ROM_CONTROL_2_INDEX 3
#define INES_RAM_BANKS_INDEX 4


void NESGamePak::ParseROMFileData(const std::vector<u8>& data)
{
	// @TODO Whole function might need some optimizations if too much
	// stuff is being copied all around the place... ?
	if (!isRomLoaded_)
	{
		assert("Tried to parse without a loaded ROM." && false);
		throw NESGamePakLoadException("Failed to parse ROM image - no ROM image loaded!");
	}

	NESReadBuffer buf(data);
	std::vector<u8> romInfo; // Vector containing the different ROM info bytes.
	try
	{
		// Read file type & support iNES files.
		// @TODO Support others?
		const auto type = buf.ReadNextStr(4);
		if (type != std::string("NES") + (char)0x1A)
			throw NESGamePakLoadException("Unexpected ROM image format!");

		// Read ROM info bytes & skip past the 7 reserved bytes.
		romInfo = buf.ReadNext(5);
		buf.ReadNext(7);

		// Read number of 8KB RAM banks. Assume 1 bank if this is 0 for compatibility reasons.
		ramBanks_ = (romInfo[INES_RAM_BANKS_INDEX] == 0 ? 1 : romInfo[INES_RAM_BANKS_INDEX]);

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
		const u8 mapperNum = ((romInfo[INES_ROM_CONTROL_2_INDEX] & 0xF0) | (romInfo[INES_ROM_CONTROL_1_INDEX] >> 4));
		switch (mapperNum)
		{
			// @TODO init mmc_
		case 0:
			//mapperType_ = NESMapperType::NROM;
			break;

		// Unknown mapper type!
		default:
			//mapperType_ = NESMapperType::UNKNOWN;
			throw NESGamePakLoadException("Unsupported ROM memory mapper!");
		}
	}
	catch (const NESReadBufferException&)
	{
		throw NESGamePakLoadException("Failed to parse ROM image header!");
	}

	try
	{
		// If there is a trainer, ignore it.
		// @TODO Do something with trainer in future?
		if (hasTrainer_)
			buf.ReadNext(512);

		// Copy contents of PRGROM and CHRROM into memory.
		for (int i = 0; i < romInfo[INES_PRGROM_BANKS_INDEX]; ++i)
		{
			NESMemPRGROMBank prgRomBank(buf.ReadNext(0x4000));
			prgRomBank.MakeReadOnly();
			prgRomBanks_.emplace_back(prgRomBank);
		}
		for (int i = 0; i < romInfo[INES_CHRROM_BANKS_INDEX]; ++i)
		{
			NESMemCHRROMBank chrRomBank(buf.ReadNext(0x2000));
			chrRomBank.MakeReadOnly();
			chrRomBanks_.emplace_back(chrRomBank);
		}
	}
	catch (const NESReadBufferException&)
	{
		throw NESGamePakLoadException("Failed to parse ROM image data!");
	}
}


void NESGamePak::LoadROM(const std::string& fileName)
{
	ResetLoadedState();

	romFileName_ = fileName;
	ParseROMFileData(ReadROMFile(fileName));

	isRomLoaded_ = true;
}


bool NESGamePak::IsROMLoaded() const
{
	return isRomLoaded_;
}


const std::vector<NESMemPRGROMBank>& NESGamePak::GetProgramROMBanks() const
{
	return prgRomBanks_;
}


const std::vector<NESMemCHRROMBank>& NESGamePak::GetCharacterROMBanks() const
{
	return chrRomBanks_;
}


NESMirroringType NESGamePak::GetMirroringType() const
{
	return mirrorType_;
}


bool NESGamePak::HasBatteryPackedRAM() const
{
	return hasBatteryPackedRam_;
}


bool NESGamePak::HasTrainer() const
{
	return hasTrainer_;
}