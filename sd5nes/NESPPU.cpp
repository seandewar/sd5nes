#include "NESPPU.h"

NESPPUMemoryMap::NESPPUMemoryMap(NESPPUMemory& mem) :
mem_(mem)
{
	// Map Pattern Tables 0 and 1.
	AddMemoryMapping(mem_.patternTables[0], 0x0000, 0x0FFF);
	AddMemoryMapping(mem_.patternTables[1], 0x1000, 0x1FFF);

	// Map Name & Attribute Tables 0 - 3.
	AddMemoryMapping(mem_.nameTables[0], 0x2000, 0x23BF);
	AddMemoryMapping(mem_.attribTables[0], 0x23C0, 0x23FF);
	AddMemoryMapping(mem_.nameTables[1], 0x2400, 0x26BF);
	AddMemoryMapping(mem_.attribTables[1], 0x27C0, 0x27FF);
	AddMemoryMapping(mem_.nameTables[2], 0x2800, 0x2BBF);
	AddMemoryMapping(mem_.attribTables[2], 0x2BC0, 0x2FBF);
	AddMemoryMapping(mem_.nameTables[3], 0x2C00, 0x2FBF);
	AddMemoryMapping(mem_.attribTables[3], 0x2FC0, 0x2FFF);

	// Mirror of $2000 to $2EFF.
	AddMemoryMirrorRange(0x2000, 0x2EFF, 0x3000, 0x3EFF);

	// Map Palette Memory.
	AddMemoryMapping(mem_.paletteMem, 0x3F00, 0x3F1F);

	// Mirror of $3F00 to $3F1F.
	AddMemoryMirrorRange(0x3F00, 0x3F1F, 0x3F20, 0x3FFF);
}


NESPPUMemoryMap::~NESPPUMemoryMap()
{
}


NESPPU::NESPPU(NESPPUMemoryMap& mem) :
mem_(mem)
{
}


NESPPU::~NESPPU()
{
}


NESPPURegisters& NESPPU::GetPPURegisters()
{
	return ppuReg_;
}
