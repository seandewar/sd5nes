#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include "NESMemory.h"
#include "NESTypes.h"
#include "NESMMC.h"
#include "NESPPU.h"

/**
* Errors relating towards the loading and parsing of ROM files.
*/
class NESGamePakLoadException : public std::runtime_error
{
public:
	explicit NESGamePakLoadException(const char* msg) :
		std::runtime_error(msg)
	{ }

	explicit NESGamePakLoadException(const std::string& msg) :
		std::runtime_error(msg)
	{ }

	virtual ~NESGamePakLoadException()
	{ }
};

/**
* Handles emulation of the NES Game Pak cartridge.
*/
class NESGamePak
{
public:
	NESGamePak();
	~NESGamePak();

	// Loads a NES ROM file. Throws NESGamePakLoadException on failure.
	void LoadROM(const std::string& fileName);

	// Returns whether or not a ROM file has been currently loaded.
	bool IsROMLoaded() const;

	// Retrieves a const reference to the read-only PRG-ROM banks contained in the cartridge.
	const std::vector<NESMemPRGROMBank>& GetProgramROMBanks() const;

	// Retrieves a const reference to the read-only CHR-ROM banks contained in the cartridge.
	const std::vector<NESMemCHRROMBank>& GetCharacterROMBanks() const;

	// Gets the mirroring type used by the ROM.
	NESPPUMirroringType GetMirroringType() const;

	// Gets the MMC @TODO Debug?
	inline NESMMC& GetMMC() const { return *mmc_; }

	// Returns whether or not the ROM has battery-packed RAM.
	bool HasBatteryPackedRAM() const;

	// Returns whether or not the ROM has a trainer included.
	bool HasTrainer() const;

private:
	bool isRomLoaded_;
	std::string romFileName_;

	/* ROM Info variables */
	NESPPUMirroringType mirrorType_;
	bool hasBatteryPackedRam_;
	bool hasTrainer_;
	u8 ramBanks_;

	/* MMC, PRG-ROM and CHR-ROM of the cart. */
	std::unique_ptr<NESMMC> mmc_;
	std::vector<NESMemPRGROMBank> prgRomBanks_;
	std::vector<NESMemCHRROMBank> chrRomBanks_;

	// Resets the state of loading.
	void ResetLoadedState();

	// Reads a ROM file.
	std::vector<u8> ReadROMFile(const std::string& fileName);

	// Parses the data from the loaded ROM file.
	void ParseROMFileData(const std::vector<u8>& data);
};

