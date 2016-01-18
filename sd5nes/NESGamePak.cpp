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

	mirrorType_ = NESNameTableMirroringType::UNKNOWN;
	hasBatteryPackedRam_ = false;
	hasTrainer_ = false;

	mmc_.reset();
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
	std::vector<u8> romInfo; // Vector containing the different ROM info bytes.
	try
	{
		// Read file type & support iNES files.
		// @TODO Support others?
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

		// Check if bit 3 is set = four screen mirroring.
		// If bit 3 is clear, check bit 0. If bit 0 is set = vertical, clear = horizontal.
		if ((romInfo[INES_ROM_CONTROL_1_INDEX] & 8) == 8)
			mirrorType_ = NESNameTableMirroringType::FOUR_SCREEN;
		else
		{
			if ((romInfo[INES_ROM_CONTROL_1_INDEX] & 1) == 1)
				mirrorType_ = NESNameTableMirroringType::VERTICAL;
			else
				mirrorType_ = NESNameTableMirroringType::HORIZONTAL;
		}

		// Check bits 1 and 2 = battery packed RAM & trainer respectively.
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
		// @TODO: Do something with trainer in future?
		if (hasTrainer_)
			buf.ReadNext(512);

		// Copy contents of PRGROM and CHRROM into memory.
		for (std::size_t i = 0; i < romInfo[INES_PRGROM_BANKS_INDEX]; ++i)
			prgBanks_.emplace_back(buf.ReadNext(0x4000));

		if (prgBanks_.size() == 0)
			throw NESGamePakLoadException("No PRG-ROM found in ROM!");

		for (std::size_t i = 0; i < romInfo[INES_CHRROM_BANKS_INDEX]; ++i)
			chrBanks_.emplace_back(buf.ReadNext(0x2000));

		// If we have no CHR-ROM banks then just create an empty one.
		// 0 = uses CHR-RAM.
		if (chrBanks_.size() == 0)
			chrBanks_.emplace_back();
	}
	catch (const NESReadBufferException&)
	{
		throw NESGamePakLoadException("Failed to parse ROM image data!");
	}

	// Get the mapper number using bits 4-7 from ROM Control Byte 1 and 2.
	const u8 mapperNum = (romInfo[INES_ROM_CONTROL_2_INDEX] & 0xF0) | (romInfo[INES_ROM_CONTROL_1_INDEX] >> 4);
	switch (mapperNum)
	{
	case 0: // NROM
		mmc_ = std::make_unique<NESMMCNROM>(
			sramBanks_[0], 
			chrBanks_[0], 
			prgBanks_[0],
			(prgBanks_.size() > 1 ? &prgBanks_[1] : nullptr)
		);
		break;

	case 1: // Nintendo MMC1 @TODO
		{
			// Create arrays of bank mappings to be used by the MMC.
			std::vector<NESMemSRAMBank*> sram;
			sram.reserve(sramBanks_.size());
			for (auto& bank : sramBanks_)
				sram.emplace_back(&bank);

			std::vector<NESMemCHRBank*> chr;
			chr.reserve(chrBanks_.size());
			for (auto& bank : chrBanks_)
				chr.emplace_back(&bank);

			std::vector<const NESMemPRGROMBank*> prg;
			prg.reserve(prgBanks_.size());
			for (auto& bank : prgBanks_)
				prg.emplace_back(&bank);

			mmc_ = std::make_unique<NESMMC1>(sram, chr, prg, mirrorType_);
		}
		break;

	// Unknown mapper type!
	default:
		std::ostringstream oss;
		oss << "Unsupported ROM memory mapper! (" << mapperNum << ")";
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


bool NESGamePak::IsROMLoaded() const
{
	return isRomLoaded_;
}


const std::vector<const NESMemPRGROMBank>& NESGamePak::GetProgramROMBanks() const
{
	return prgBanks_;
}


const std::vector<NESMemCHRBank>& NESGamePak::GetCharacterBanks() const
{
	return chrBanks_;
}


NESNameTableMirroringType NESGamePak::GetMirroringType() const
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