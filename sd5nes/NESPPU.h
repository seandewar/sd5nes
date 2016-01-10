#pragma once

#include <array>
#include <iostream> // @TODO: DEBUG

#include <SFML\Graphics\Color.hpp>
#include <SFML\Graphics\Image.hpp>

#include "NESTypes.h"
#include "NESHelper.h"
#include "NESMemory.h"

/* Typedefs for the individual tables + typedef for holding both palettes (BG and Sprite). */
typedef NESMemory<0x400> NESMemNameTable;
typedef NESMemory<0x20> NESMemPalettes;

/**
* Struct containing palette memory, name and attribute tables for the PPU.
*/
struct NESPPUMemory
{
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
* Struct for PPU Color values.
*/
struct NESPPUColor
{
	u8 r, g, b;

	NESPPUColor() :
		r(0), g(0), b(0)
	{ }

	NESPPUColor(u8 r, u8 g, u8 b) :
		r(r), g(g), b(b)
	{ }

	/**
	* Converts an NESPPUColor into it's corrisponding SFML Color (sf::Color).
	*/
	inline sf::Color ToSFColor() const { return sf::Color(r, g, b); }
};

/**
* Read-only array containing the 64 NES PPU Palette colours indexed approperiately.
*/
const std::array<NESPPUColor, 0x40> ppuPalette = {
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
* The probability of certain bits in PPUSTATUS being set on power-on state.
*/
#define NES_PPU_POWER_REG_PPUSTATUS_V_SET_CHANCE 0.75
#define NES_PPU_POWER_REG_PPUSTATUS_O_SET_CHANCE 0.75

/**
* How many CPU cycles writes to PPUCTRL, PPUMASK, PPUSCROLL and PPUADDR should be ignored for
* after a reset.
*
* Because of this, the address latch cannot toggle for the duration of this event.
*/
#define NES_PPU_RESET_REG_IGNORE_WRITE_FOR_CPU_CYC 29658

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

	/* Amount of cycles left to ignore writes to PPUCTRL, PPUMASK, PPUSCROLL and PPUADDR. */
	unsigned int writeIgnoreCyclesLeft;

	NESPPURegisters() :
		PPUCTRL(0), PPUMASK(0), PPUSTATUS(0),
		OAMADDR(0),
		writeIgnoreCyclesLeft(0)
	{ }
};

/* Specifies an invalid index inside of OAM. */
#define NES_INVALID_OAM_INDEX -1

/**
* Internal structure of a stored sprite
* to be rendered for the next scanline.
*/
struct NESPPUSprite
{
	u8 x, y, tileIndex, attributes;
	u8 tileBitmapHi, tileBitmapLo;

	NESPPUSprite(u8 primaryOamIndex = NES_INVALID_OAM_INDEX) :
		primaryOamIndex_(primaryOamIndex),
		x(0), y(0), tileIndex(0), attributes(0),
		tileBitmapHi(0), tileBitmapLo(0)
	{ }

	/**
	* Gets the original index in primary OAM of the Sprite.
	*/
	inline u8 GetPrimaryOAMIndex() const { return primaryOamIndex_; }

private:
	u8 primaryOamIndex_;
};

/**
* Internal structure of a stored background tile's data.
*/
struct NESPPUBGTileData
{
	u8 ntByte, atByte;
	u8 tileBitmapHi, tileBitmapLo;

	NESPPUBGTileData() :
		ntByte(0), atByte(0),
		tileBitmapHi(0), tileBitmapLo(0)
	{ }

	NESPPUBGTileData(u8 nt, u8 at, u8 tileBmpHi, u8 tileBmpLo) :
		ntByte(nt), atByte(at), 
		tileBitmapHi(tileBmpHi), tileBitmapLo(tileBmpLo)
	{ }
};

/* The amount of PPU cycles it takes for the value inside the internal data bus to decay. */
#define NES_PPU_DATA_BUS_DECAY_CYCLES 357368

/**
* Struct containing info on the status of the PPU's reg write latches.
*/
struct NESPPULatches
{
	/* The value stored in the internal data bus. */
	u8 internalDataBusVal;

	/* Amount of cycles left until the value in the internal data bus decays away. */
	unsigned int cyclesLeftUntilBusDecay;

	/* The state of the address latch used by PPUSCROLL and PPUADDR. */
	bool isAddressLatchOn;

	NESPPULatches() :
		internalDataBusVal(0),
		cyclesLeftUntilBusDecay(0),
		isAddressLatchOn(false)
	{ }
};

/**
* Interface for allowing the PPU to communicate with other devices.
*/
class INESPPUCommunicationsInterface : public INESMemoryInterface
{
public:
	virtual ~INESPPUCommunicationsInterface() { }

	virtual void PullNMI() = 0;
	virtual std::array<u8, 0x100> OAMDMARead(u8 addrPage) = 0;
};

/**
* Handles emulation of the 2C02 PPU used in the NES.
*/
class NESPPU
{
public:
	explicit NESPPU(sf::Image& debug);
	~NESPPU();

	/**
	* Debug : Draws sprites from the pattern table.
	*/
	void DebugDrawPatterns(sf::Image& target, int colorOffset);

	/**
	* Initialize the PPU.
	*/
	void Initialize(INESPPUCommunicationsInterface& comm);

	/**
	* Sets the PPU to its power-up state.
	*/
	void Power();

	/**
	* Sets the PPU to its reset state.
	*/
	void Reset();

	/**
	* Tick the PPU for one cycle.
	*/
	void Tick();

	/**
	* Writes to the specified PPU register.
	*/
	void WriteRegister(NESPPURegisterType reg, u8 val);

	/**
	* Reads from the specified PPU register.
	*/
	u8 ReadRegister(NESPPURegisterType reg);

	/**
	* Returns whether or not rendering is enabled.
	* (Rendering is disabled if bits 3 and 4 in PPUMASK are cleared).
	*/
	inline bool IsRenderingEnabled() const { return ((reg_.PPUMASK & 0x18) != 0); }

	/**
	* Whether or not the frame has completely finished rendering on the last tick.
	*/
	inline bool IsFrameFinished() const { return isFrameFinished_; }

private:
	sf::Image& debug_; // @TODO DEBUG!!

	INESPPUCommunicationsInterface* comm_;

	NESPPURegisters reg_;
	NESPPULatches latches_;

	NESMemory<0x100> primaryOam_;
	NESMemory<0x20> secondaryOam_;

	bool isFrameFinished_;

	unsigned int elapsedCycles_;

	unsigned int currentScanline_;
	unsigned int currentCycle_;

	bool isNmiPulled_;
	bool xIncdThisTick_, yIncdThisTick_;

	// v (current v-ram addr) [15-bits used], t (temp v-ram addr) [15-bits used]
	// x (fine x scroll) [3-bits used]
	u16 vScroll_, tScroll_;
	u8 xScroll_;

	// Buffered data of PPUDATA.
	u8 ppuDataBuffered_;

	std::array<NESPPUBGTileData, 2> activeTiles_;
	NESPPUBGTileData bufferingTile_;

	u8 activeSpriteCount_;
	std::array<NESPPUSprite, 8> activeSprites_;

	bool isEvenFrame_;

	/**
	* Gets the height of sprites as defined in H in PPUCTRL.
	*/
	inline u8 GetSpriteHeight() const
	{
		return (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_H_BIT) ? 16u : 8u);
	}

	/**
	* Gets the address of a background tile's (low) bitmap
	* using its tileIndex.
	*/
	inline u16 GetBackgroundTileAddress(u8 tileIndex) const
	{
		return (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_B_BIT) ? 0x1000 : 0) + (tileIndex * 16);
	}

	/**
	* Gets the address of a sprite's (low) tile bitmap
	* from its tileIndex.
	*/
	inline u16 GetSpriteTileAddress(u8 tileIndex) const
	{
		if (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_H_BIT))
			return (NESHelper::IsBitSet(tileIndex, 0) ? 0x1000 : 0) + (tileIndex * 32);
		else
			return (NESHelper::IsBitSet(reg_.PPUCTRL, NES_PPU_REG_PPUCTRL_S_BIT) ? 0x1000 : 0) + (tileIndex * 16);
	}

	/**
	* Gets the color of a tile bitmap line's pixel at a specified position.
	* pos cannot be above 7.
	*/
	inline u8 GetTileBitmapLinePixel(u8 tileBmpHi, u8 tileBmpLo, u8 pos)
	{
		assert(pos < 8);

		return (((tileBmpHi >> (7 - pos)) & 1) << 1) | (tileBmpLo >> (7 - pos)) & 1;
	}

	/**
	* Gets a tile bitmap line while respecting horizontal and vertical flipping.
	* Also supports tiles for sprites of 16 height.
	* lineNum should be in the range 0 to 15.
	*/
	inline u8 FetchTileBitmapLine(u16 tileAddr, u8 lineNum, bool flipHoriz, bool flipVert) const
	{
		assert(lineNum < 16);

		//const u16 tileBmpAddr = tileAddr + (lineNum >= 8 ? 8 : 0) + (flipVert ? 7 - lineNum : lineNum);
		const u16 tileBmpAddr = tileAddr + (flipVert ? (lineNum >= 8 ? 15 : 7) - lineNum : lineNum);
		const u8 tileBitmapLine = comm_->Read8(tileBmpAddr);
		return (flipHoriz ? NESHelper::ReverseBits(tileBitmapLine) : tileBitmapLine);
	}

	/**
	* Increments X of v.
	* Result of the increment overflows into horizontal nt select of v.
	*/
	void IncrementScrollX();

	/**
	* Increments Y of v.
	* Result of the increment overflows into coarse Y of v.
	* Will wrap between the vertical name tables.
	*/
	void IncrementScrollY();

	/**
	* Increments VRAM Address after accessing PPUDATA
	* and emulates the strange result of accessing PPUDATA 
	* while the PPU is rendering.
	*/
	void HandlePPUDATAAccess();

	/**
	* Handles the fetching of tile data for this tick.
	*/
	void TickFetchTileData();

	/**
	* Handles the evaluation of sprites for this tick.
	*/
	void TickEvaluateSprites();

	/**
	* Handles the rendering of pixels for this tick.
	*/
	void TickRenderPixel();
};

