#pragma once

#include <string>
#include <vector>

#include "NESMemory.h"

/**
* Handles emulation of the NES Game Pak cartridge.
*/
class NESGamePak
{
public:
	NESGamePak();
	~NESGamePak();

	// Loads a NES ROM file.
	bool LoadROM(const std::string& fileName);

	// Returns whether or not a ROM file has been currently loaded.
	bool IsROMLoaded() const;

	// Retrieves a const reference to the read-only PRG-ROM contained in the cartridge.
	const NESMemoryPRGROM& GetProgramROM() const;

	// Retrieves a const reference to the read-only CHR-ROM contained in the cartridge.
	const NESMemoryCHRROM& GetCharacterROM() const;

private:
	bool isRomLoaded_;
	std::string romFileName_;
	std::vector<u8> romFileData_;

	NESMemoryPRGROM prgRom_;
	NESMemoryCHRROM chrRom_;

	// Reads a ROM file.
	bool ReadROMFile(const std::string& fileName);

	// Parses the data from the loaded ROM file.
	bool ParseROMFileData();
};

