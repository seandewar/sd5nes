#include "NESPPU.h"

NESPPUMemoryMap::NESPPUMemoryMap(
	const std::array<NESMemory&, 2>& patternTables,
	const std::array<NESMemory&, 4>& nameTables,
	const std::array<NESMemory&, 4>& attribTables,
	NESMemory& paletteMem
) :
	patternTables_(patternTables),
	nameTables_(nameTables),
	attribTables_(attribTables),
	paletteMem_(paletteMem)
{
	// Map Pattern Tables 0 and 1.
	AddMemoryMapping(patternTables_[0], 0x0000, 0x0FFF);
	AddMemoryMapping(patternTables_[1], 0x1000, 0x1FFF);

	// Map Name & Attribute Tables 0 - 3.
	AddMemoryMapping(nameTables_[0], 0x2000, 0x23BF);
	AddMemoryMapping(attribTables_[0], 0x23C0, 0x23FF);
	AddMemoryMapping(nameTables_[1], 0x2400, 0x26BF);
	AddMemoryMapping(attribTables_[1], 0x27C0, 0x27FF);
	AddMemoryMapping(nameTables_[2], 0x2800, 0x2BBF);
	AddMemoryMapping(attribTables_[2], 0x2BC0, 0x2FBF);
	AddMemoryMapping(nameTables_[3], 0x2C00, 0x2FBF);
	AddMemoryMapping(attribTables_[3], 0x2FC0, 0x2FFF);

	// Mirror of $2000 to $2EFF.
	AddMemoryMirrorRange(0x2000, 0x2EFF, 0x3000, 0x3EFF);

	// Map Palette Memory.
	AddMemoryMapping(paletteMem_, 0x3F00, 0x3F1F);

	// Mirror of $3F00 to $3F1F.
	AddMemoryMirrorRange(0x3F00, 0x3F1F, 0x3F20, 0x3FFF);
}


NESPPUMemoryMap::~NESPPUMemoryMap()
{
}


NESPPU::NESPPU(NESMemory& mem) :
mem_(mem)
{
}


NESPPU::~NESPPU()
{
}


