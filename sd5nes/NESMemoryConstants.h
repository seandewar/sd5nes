#pragma once

/* Area of mem holding the Stack. */
#define NES_MEMORY_STACK_START 0x0100
#define NES_MEMORY_STACK_END 0x01FF

/* Area of mem holding Expansion ROM mappings. */
#define NES_MEMORY_EXPANSION_ROM_START 0x4020
#define NES_MEMORY_EXPANSION_ROM_END 0x5FFF

/* Area of mem mapping PPU registers */
#define NES_MEMORY_PPU_REGISTER_START 0x2000
#define NES_MEMORY_PPU_REGISTER_END 0x2007

/* Area of mem holding SRAM. */
#define NES_MEMORY_SRAM_START 0x6000
#define NES_MEMORY_SRAM_END 0x7FFF

/* Area of mem containing Game Pak data. */
#define NES_MEMORY_GAME_PAK_START 0x8000