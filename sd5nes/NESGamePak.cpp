#include "NESGamePak.h"

#include <fstream>


NESGamePak::NESGamePak() :
isRomLoaded_(false)
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
		// Loading succeeded.
		isRomLoaded_ = true;
		romFileName_ = fileName;
		romFileData_ = fileData;
		return true;
	}

	// If not EOF, then loading failed.
	return false;
}


bool NESGamePak::ParseROMFileData()
{
	if (!isRomLoaded_)
		return false;

	// @ TODO Parse as iNES?
	return true;
}


bool NESGamePak::LoadROM(const std::string& fileName)
{
	if (!ReadROMFile(fileName))
		return false;

	return ParseROMFileData();
}


bool NESGamePak::IsROMLoaded() const
{
	return isRomLoaded_;
}


const NESMemoryPRGROM& NESGamePak::GetProgramROM() const
{
	return prgRom_;
}


const NESMemoryCHRROM& NESGamePak::GetCharacterROM() const
{
	return chrRom_;
}