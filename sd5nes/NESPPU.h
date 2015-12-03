#pragma once

#include <array>

#include <SFML\Graphics\Color.hpp>
#include <SFML\Graphics\Image.hpp>

#include "NESTypes.h"
#include "NESMemory.h"

/* Typedefs for the individual tables + typedef for holding both palettes (BG and Sprite). */
typedef NESMemory<0x1000> NESMemPatternTable;
typedef NESMemory<0x400> NESMemNameTable;
typedef NESMemory<0x20> NESMemPalettes;

/**
* Struct containing palette memory, pattern, name and attribute tables for the PPU.
*/
struct NESPPUMemory
{
	std::array<NESMemPatternTable, 2> patternTables;
	std::array<NESMemNameTable, 4> nameTables;
	NESMemPalettes paletteMem;
};

/**
* Emulates the mapping and mirroring of the PPU's memory.
*/
class NESPPUMemoryMapper : public NESMemoryMapper
{
public:
	NESPPUMemoryMapper(NESPPUMemory& mem);
	virtual ~NESPPUMemoryMapper();

protected:
	std::pair<INESMemoryInterface*, u16> GetMapping(u16 addr) const override;

private:
	NESPPUMemory& mem_;
};

/**
* Struct for PPU Color values.
*/
struct NESPPUColor
{
	u8 r, g, b;

	// Converts an NESPPUColor into it's corrisponding SFML Color (sf::Color).
	inline sf::Color ToSFColor() const { return sf::Color(r, g, b); }
};

/**
* Read-only array containing the 64 NES PPU Palette colours indexed approperiately.
*/
const std::array<NESPPUColor, 0x40> NES_PPU_PALETTE_COLORS = {
	{
		{ 0x75, 0x75, 0x75 }, { 0x27, 0x1B, 0x8F }, { 0x00, 0x00, 0xAB }, { 0x47, 0x00, 0x9F },
		{ 0x8F, 0x00, 0x77 }, { 0xAB, 0x00, 0x13 }, { 0xA7, 0x00, 0x00 }, { 0x7F, 0x0B, 0x00 },
		{ 0x43, 0x2F, 0x00 }, { 0x00, 0x47, 0x00 }, { 0x00, 0x51, 0x00 }, { 0x00, 0x3F, 0x17 },
		{ 0x1B, 0x3F, 0x5F }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 },
		{ 0xBC, 0xBC, 0xBC }, { 0x00, 0x73, 0xEF }, { 0x23, 0x3B, 0xEF }, { 0x83, 0x00, 0xF3 },
		{ 0xBF, 0x00, 0xBF }, { 0xE7, 0x00, 0x5B }, { 0xDB, 0x2B, 0x00 }, { 0xCB, 0x4F, 0x0F },
		{ 0x8B, 0x73, 0x00 }, { 0x00, 0x97, 0x00 }, { 0x00, 0xAB, 0x00 }, { 0x00, 0x93, 0x3B },
		{ 0x00, 0x83, 0x8B }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 },
		{ 0xFF, 0xFF, 0xFF }, { 0x3F, 0xBF, 0xFF }, { 0x5F, 0x97, 0xFF }, { 0xA7, 0x8B, 0xFD },
		{ 0xF7, 0x7B, 0xFF }, { 0xFF, 0x77, 0xB7 }, { 0xFF, 0x77, 0x63 }, { 0xFF, 0x9B, 0x3B },
		{ 0xF3, 0xBF, 0x3F }, { 0x83, 0xD3, 0x13 }, { 0x4F, 0xDF, 0x4B }, { 0x58, 0xF8, 0x98 },
		{ 0x00, 0xEB, 0xDB }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 },
		{ 0xFF, 0xFF, 0xFF }, { 0xAB, 0xE7, 0xFF }, { 0xC7, 0xD7, 0xFF }, { 0xD7, 0xCB, 0xFF },
		{ 0xFF, 0xC7, 0xFF }, { 0xFF, 0xC7, 0xDB }, { 0xFF, 0xBF, 0xB3 }, { 0xFF, 0xDB, 0xAB },
		{ 0xFF, 0xE7, 0xA3 }, { 0xE3, 0xFF, 0xA3 }, { 0xAB, 0xF3, 0xBF }, { 0xB3, 0xFF, 0xCF },
		{ 0x9F, 0xFF, 0xF3 }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 }
	}
};

/**
* Struct containing the registers used by the NES PPU.
*/
struct NESPPURegisters
{
	union
	{
		/* PPU Control Register 1 (PPUCTRL0) */
		u8 PPUCTRL0;

		u8 nameTableNum : 2; /* Name table number. */
		u8 incVramAddr : 1; /* Increments VRAM address by 1 if this is 0, otherwise by 32 if this is 1. */
		u8 spritePatternTableNum : 1; /* Number of the Pattern table that the sprites are stored in. */
		u8 bgPatternTableNum : 1; /* Number of the Pattern table that the background is stored in. */
		u8 spritePixelSize : 1; /* Size of the sprites in pixels - 8x8 if this is 0, otherwise 8x16 if this is 1. */
		u8 PPUCTRL0Unused : 1; /* Unused bit. */
		u8 vBlankTriggerNMI : 1; /* Whether or not an NMI should be triggered upon a V-Blank. */
	};

	union
	{
		/* PPU Control Register 2 (PPUCTRL1) */
		u8 PPUCTRL1;

		u8 colorMode : 1; /* Indicates colour (0) or monochrome mode (1). */
		u8 clipBg : 1; /* Whether or not to clip the background (AKA hide the background in the left 8 pixels on the screen). */
		u8 clipSprites : 1; /* Whether or not to clip the sprites (AKA hide the sprites in the left 8 pixels on the screen). */
		u8 displayBg : 1; /* Does not display the background if 0. */
		u8 displaySprites : 1; /* Does not display sprites if 0. */
		u8 colorModeInfo : 3; /* Indicates background colour in monochrome mode or colour intensity in colour mode. */
	};

	union
	{
		/* Status Register (PPUSTAT) */
		u8 PPUSTAT;

		u8 PPUSTATUnused : 4; /* Unused bits. */
		u8 ignoreVramWrites : 1; /* If set to 1, writes to VRAM will be ignored. */
		u8 scanlineSpritesCount : 1; /* If set to 1, indicates that more than 8 sprites are on the current scanline. */
		u8 sprite0Hit : 1; /* Set when a non-transparent pixel of Sprite 0 overlaps a non-transparent background pixel. */
		u8 vBlankInProgress : 1; /* Whether or not a V-Blank is in progress. */
	};

	u8 sprRamAddr; /* The address in SPR-RAM to access on the next write to sprRamIO. */
	u8 sprRamIO; /* Writes a byte to SPR-RAM at the address indicated by sprRamAddr. */

	u8 vramAddr1; /* VRAM address register 1 */
	u8 vramAddr2; /* VRAM address register 2 */

	u8 vramIO; /* Writes a byte to VRAM at the address indicated by vramAddr. */
};

/**
* Handles emulation of the 2C02 PPU used in the NES.
*/
class NESPPU
{
public:
	explicit NESPPU(NESPPUMemoryMapper& mem);
	~NESPPU();

	/**
	* Ticks the PPU.
	*/
	void Tick();

	/**
	* Debug : Draws sprites from the pattern table.
	*/
	void DebugDrawPatterns(sf::Image& target, int colorOffset);

	/**
	* Gets a reference to the registers being used by the PPU.
	*/
	NESPPURegisters& GetPPURegisters();

private:
	NESPPURegisters ppuReg_;
	NESPPUMemoryMapper& mem_;

	bool isEvenFrame_;
	int currentScanline_;

	/**
	* Inits the PPU.
	*/
	void Initialize();

	/**
	* Handles the logic and rendering of the current scanline.
	*/
	void HandleScanline();
};

