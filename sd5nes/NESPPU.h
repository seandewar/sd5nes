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
	std::array<NESMemNameTable, 2> nameTables;
	NESMemPalettes paletteMem;
};

/**
* The types of nametable mirroring that can be used.
*/
enum class NESPPUMirroringType
{
	HORIZONTAL,
	VERTICAL,
	ONE_SCREEN,
	FOUR_SCREEN,
	UNKNOWN
};

/**
* Emulates the mapping and mirroring of the PPU's memory.
*/
class NESPPUMemoryMapper : public NESMemoryMapper
{
public:
	NESPPUMemoryMapper(NESPPUMemory& mem, NESPPUMirroringType ntMirror);
	virtual ~NESPPUMemoryMapper();

protected:
	std::pair<INESMemoryInterface*, u16> GetMapping(u16 addr) const override;

private:
	NESPPUMemory& mem_;
	NESPPUMirroringType ntMirror_;
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

/* Positions of the different bits in the OAM Attribute field. */
#define NES_PPU_OAM_ATTRIB_PALETTE_HI_BIT 0
#define NES_PPU_OAM_ATTRIB_PALETTE_LO_BIT 1
#define NES_PPU_OAM_ATTRIB_PRORITY_BIT 5
#define NES_PPU_OAM_ATTRIB_HFLIP_BIT 6
#define NES_PPU_OAM_ATTRIB_VFLIP_BIT 7

/**
* Represents 1 entry in the PPU's OAM (Object Attribute Memory).
*/
struct NESPPUOAMEntry
{
	/* Y position of the top of the sprite. */
	u8 spriteY;

	/* Index number of the tile to use for this sprite. */
	u8 tileIndex;

	/* Sprite attributes. */
	u8 attrib;
};

/* Positions of the different PPUCTRL bits. */
#define NES_PPU_REG_PPUCTRL_N_HI_BIT 0
#define NES_PPU_REG_PPUCTRL_N_LO_BIT 1
#define NES_PPU_REG_PPUCTRL_I_BIT 2
#define NES_PPU_REG_PPUCTRL_S_BIT 3
#define NES_PPU_REG_PPUCTRL_B_BIT 4
#define NES_PPU_REG_PPUCTRL_H_BIT 5
#define NES_PPU_REG_PPUCTRL_P_BIT 6
#define NES_PPU_REG_PPUCTRL_V_BIT 7

/* Positions of the different PPUMASK bits. */
#define NES_PPU_REG_PPUMASK_G_BIT 0
#define NES_PPU_REG_PPUMASK_m_BIT 1
#define NES_PPU_REG_PPUMASK_M_BIT 2
#define NES_PPU_REG_PPUMASK_b_BIT 3
#define NES_PPU_REG_PPUMASK_s_BIT 4
#define NES_PPU_REG_PPUMASK_BGR_HI_BIT 5
#define NES_PPU_REG_PPUMASK_BGR_MID_BIT 6
#define NES_PPU_REG_PPUMASK_BGR_LO_BIT 7

/* Positions of the different PPUSTATUS bits. */
#define NES_PPU_REG_PPUSTATUS_O_BIT 5
#define NES_PPU_REG_PPUSTATUS_S_BIT 6
#define NES_PPU_REG_PPUSTATUS_V_BIT 7

/**
* Enum containing a list of the different types of registers.
*/
enum class NESPPURegisterType
{
	PPUCTRL,
	PPUMASK,
	PPUSTATUS,
	OAMADDR,
	OAMDATA,
	PPUSCROLL,
	PPUADDR,
	PPUDATA,
	OAMDMA,
	UNKNOWN
};

/**
* Struct containing the registers used by the NES PPU.
*/
struct NESPPURegisters
{
	/* PPU Control Register 1 (PPUCTRL) */
	u8 PPUCTRL;

	/* PPU Control Register 2 (PPUMASK) */
	u8 PPUMASK;

	/* Status Register (PPUSTATUS) */
	u8 PPUSTATUS;

	/* OAM Read/Write Address (OAMADDR) */
	u8 OAMADDR; 

	/* OAM Data Read/Write (OAMDATA) */
	u8 OAMDATA;

	/* Scroll Position (PPUSCROLL) W x2 */
	u8 PPUSCROLL;

	/* PPU Read/Write Address (PPUADDR) W x2 */
	u8 PPUADDR;

	/* PPU Data Read/Write (PPUDATA) */
	u8 PPUDATA;

	/* OAM DMA High Address (OAMDMA) */
	u8 OAMDMA;
};

/**
* Struct containing info on the status of the PPU's reg write latches.
*/
struct NESPPULatches
{
	/* The value stored in the internal data bus. */
	u8 internalDataBusVal;

	/* The state of the address latch used by PPUSCROLL and PPUADDR. */
	bool isAddressLatchOn;
};

/**
* Handles emulation of the 2C02 PPU used in the NES.
*/
class NESPPU
{
public:
	explicit NESPPU(NESPPUMemoryMapper& mem, sf::Image& debug);
	~NESPPU();

	/**
	* Handles one frame worth of PPU logic.
	*/
	void Frame();

	/**
	* Debug : Draws sprites from the pattern table.
	*/
	void DebugDrawPatterns(sf::Image& target, int colorOffset);

	/**
	* Writes to the specified PPU register.
	*/
	void WriteRegister(NESPPURegisterType reg, u8 val);

	/**
	* Reads from the specified PPU register.
	*/
	u8 ReadRegister(NESPPURegisterType reg);

private:
	sf::Image& debug_; // @TODO DEBUG!!

	NESPPURegisters reg_;
	NESPPULatches latches_;
	u8 xNtScroll_, yNtScroll_;
	u16 vramDataAddr_;

	NESPPUMemoryMapper& mem_;

	NESMemory<0x100> primaryOam_;
	NESMemory<0x20> secondaryOam_;

	bool isEvenFrame_;

	/**
	* Inits the PPU.
	*/
	void Initialize();

	/**
	* Handles the logic and rendering of the current scanline.
	*/
	void HandleScanline(unsigned int scanline);
};

