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

	mirrorType_ = NESPPUMirroringType::UNKNOWN;
	hasBatteryPackedRam_ = false;
	hasTrainer_ = false;
	ramBanks_ = 0;

	sram_.ZeroMemory(); // @TODO: Support saving...

	mmc_.reset();
	romFileName_.clear();
	prgRomBanks_.clear();
	chrBanks_.clear();
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

		// Read number of 8KB RAM banks. Assume 1 bank if this is 0 for compatibility reasons.
		ramBanks_ = (romInfo[INES_RAM_BANKS_INDEX] == 0 ? 1 : romInfo[INES_RAM_BANKS_INDEX]);

		// Check if bit 3 is 1 = four screen mirroring.
		// If bit 3 is 0, check bit 0. If bit 0 is 1 = vertical. 0 = horizontal.
		if ((romInfo[INES_ROM_CONTROL_1_INDEX] & 8) == 8)
			mirrorType_ = NESPPUMirroringType::FOUR_SCREEN;
		else
			mirrorType_ = ((romInfo[INES_ROM_CONTROL_1_INDEX] & 1) == 1 ? NESPPUMirroringType::VERTICAL : NESPPUMirroringType::HORIZONTAL);

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
		for (int i = 0; i < romInfo[INES_PRGROM_BANKS_INDEX]; ++i)
			prgRomBanks_.emplace_back(NESMemPRGROMBank(buf.ReadNext(0x4000)));
		for (int i = 0; i < romInfo[INES_CHRROM_BANKS_INDEX]; ++i)
			chrBanks_.emplace_back(NESMemCHRBank(buf.ReadNext(0x2000)));

		// If we have no CHR-ROM/RAM banks then just create an empty one.
		if (chrBanks_.size() == 0)
			chrBanks_.emplace_back(NESMemCHRBank());
	}
	catch (const NESReadBufferException&)
	{
		throw NESGamePakLoadException("Failed to parse ROM image data!");
	}

	// Get the mapper number using bits 4-7 from ROM Control Byte 1 and 2.
	const u8 mapperNum = (romInfo[INES_ROM_CONTROL_2_INDEX] & 0xF0) | (romInfo[INES_ROM_CONTROL_1_INDEX] >> 4);
	switch (mapperNum)
	{
	case 0:
		if (prgRomBanks_.size() == 0)
			throw NESGamePakLoadException("No PRG-ROM banks for NROM mapper!");

		mmc_ = std::make_unique<NESMMCNROM>(
			sram_, 
			chrBanks_[0], 
			prgRomBanks_[0],
			(prgRomBanks_.size() > 1 ? &prgRomBanks_[1] : nullptr)
		);
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
	return prgRomBanks_;
}


const std::vector<NESMemCHRBank>& NESGamePak::GetCharacterBanks() const
{
	return chrBanks_;
}


NESPPUMirroringType NESGamePak::GetMirroringType() const
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