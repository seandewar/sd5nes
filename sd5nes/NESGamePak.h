#pragma once

#include <string>
#include <vector>
#include <memory>

#include "NESException.h"
#include "NESMemory.h"
#include "NESTypes.h"
#include "NESGamePakPowerState.h"

/**
* Errors relating towards the loading and parsing of ROM files.
*/
class NESGamePakLoadException : public NESException
{
public:
	explicit NESGamePakLoadException(const char* msg) : NESException(msg) { }
	explicit NESGamePakLoadException(const std::string& msg) : NESException(msg) { }
	virtual ~NESGamePakLoadException() { }
};

/**
* Handles emulation of the NES Game Pak cartridge.
*/
class NESGamePak
{
public:
	NESGamePak();
	~NESGamePak();

	/**
	* Loads a NES ROM file. Throws NESGamePakLoadException on failure.
	*/
	void LoadROM(const std::string& fileName);

	/**
	* Returns whether or not a ROM file has been currently loaded.
	*/
	inline bool IsROMLoaded() const { return isRomLoaded_; }

	/**
	* Creates a new Game Pak power state for this Game Pak, and returns ownership of it.
	*/
	std::unique_ptr<NESGamePakPowerState> GetNewGamePakPowerState() const;

	/**
	* Retrieves a const reference to the read-only PRG-ROM banks contained in the cartridge.
	*/
	inline const std::vector<NESMemPRGROMBank>& GetProgramROMBanks() const { return prgBanks_; }

	/**
	* The loaded CHR-ROM / CHR-RAM banks contained in the cartridge.
	*/
	inline const std::vector<NESMemCHRBank>& GetCharacterBanks() const { return chrBanks_; }

	/**
	* Gets the current mirroring type used by the ROM.
	*/
	inline NESNameTableMirroringType GetMirroringType() const { return mirrorType_; }

	/**
	* Returns whether or not the ROM has battery-packed RAM.
	*/
	inline bool HasBatteryPackedRAM() const { return hasBatteryPackedRam_; }

	/**
	* Returns whether or not the ROM has a trainer included.
	*/
	inline bool HasTrainer() const { return hasTrainer_; }

private:
	bool isRomLoaded_;
	std::string romFileName_;

	/* ROM Info variables */
	NESMMCType mapperType_;
	NESNameTableMirroringType mirrorType_;
	bool hasBatteryPackedRam_;
	bool hasTrainer_;

	/* Loaded SRAM, PRG-ROM and CHR-ROM of the cart. */
	std::vector<NESMemPRGROMBank> prgBanks_;
	std::vector<NESMemCHRBank> chrBanks_;
	std::vector<NESMemSRAMBank> sramBanks_;

	/**
	* Resets the state of loading.
	*/
	void ResetLoadedState();

	/**
	* Reads a ROM file.
	*/
	std::vector<u8> ReadROMFile(const std::string& fileName);

	/**
	* Parses the data from the loaded ROM file.
	*/
	void ParseROMFileData(const std::vector<u8>& data);
};

