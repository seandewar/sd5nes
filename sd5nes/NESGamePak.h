#pragma once

#include <string>
#include <vector>

#include "NESMemory.h"

/**
* The type of mirroring used by the ROM.
*/
enum class NESMirroringType
{
	HORIZONTAL,
	VERTICAL,
	FOUR_SCREEN,
	UNKNOWN
};

/**
* The type of mapper used by the ROM.
*/
enum class NESMapperType : u8
{
	NROM, // No mapper.
	UNKNOWN
};

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
	const NESMemory& GetProgramROM() const;

	// Retrieves a const reference to the read-only CHR-ROM contained in the cartridge.
	const NESMemory& GetCharacterROM() const;

	// Gets the mirroring type used by the ROM.
	NESMirroringType GetMirroringType() const;

	// Gets the mapper type used by the ROM.
	NESMapperType GetMapperType() const;

	// Returns whether or not the ROM has battery-packed RAM.
	bool HasBatteryPackedRAM() const;

	// Returns whether or not the ROM has a trainer included.
	bool HasTrainer() const;

private:
	bool isRomLoaded_;
	std::string romFileName_;
	std::vector<u8> romFileData_;

	/* ROM Info variables */
	NESMirroringType mirrorType_;
	NESMapperType mapperType_;
	bool hasBatteryPackedRam_;
	bool hasTrainer_;

	/* PRG-ROM and CHR-ROM of the cart. */
	NESMemory prgRom_;
	NESMemory chrRom_;

	// Reads a ROM file.
	bool ReadROMFile(const std::string& fileName);

	// Parses the data from the loaded ROM file.
	bool ParseROMFileData();
};

