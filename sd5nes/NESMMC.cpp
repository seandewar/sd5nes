#include "NESMMC.h"


NESMMCNROM::NESMMCNROM(NESMemSRAMBank& sram, 
	NESMemCHRBank& chr, 
	const NESMemPRGROMBank& prg1, 
	const NESMemPRGROMBank* prg2) :
sram_(sram),
chr_(chr)
{
	prg_[0] = &prg1;
	prg_[1] = (prg2 != nullptr ? prg2 : &prg1);
}


NESMMCNROM::~NESMMCNROM()
{
}


void NESMMCNROM::Write8(u16 addr, u8 val)
{
	if (addr < 0x2000) // CHR-ROM / CHR-RAM
		chr_.Write8(addr, val);
	else if (addr >= 0x6000 && addr < 0x8000) // SRAM
		sram_.Write8(addr - 0x6000, val);
}


u8 NESMMCNROM::Read8(u16 addr) const
{
	if (addr < 0x2000) // CHR-ROM / CHR-RAM
		return chr_.Read8(addr);
	else if (addr >= 0x6000 && addr < 0x8000) // SRAM
		return sram_.Read8(addr - 0x6000);
	else if (addr >= 0x8000) // Upper & Lower PRG-ROM Banks
		return prg_[(addr - 0x8000) / 0x4000]->Read8(addr & 0x3FFF);
	else
		return 0;
}


NESMMC1::NESMMC1(const std::vector<NESMemSRAMBank>& sram, 
	const std::vector<NESMemCHRBank>& chr,
	const std::vector<NESMemPRGROMBank>& prg,
	NESNameTableMirroringType& ntMirror) :
sram_(sram),
chr_(chr),
prg_(prg),
ntMirror_(ntMirror),
shiftReg_(0x10),
prgBankMode_(0),
chrBankMode_(0),
chrBankIndices_({ { 0, chr_.size() - 1 } }),
prgBankIndices_({ { 0, prg_.size() - 1 } })
{
	// Ensure that we have at least one valid bank in SRAM, CHR and PRG.
	// and that we do not have more banks than the mapper can use.
	assert(sram_.size() != 0 && chr_.size() != 0 && prg_.size() != 0);
	assert(sram_.size() <= 4 && chr_.size() <= 16 && prg_.size() <= 32);
}


NESMMC1::~NESMMC1()
{
}


void NESMMC1::WriteControlRegister(u8 val)
{
	// Modify mirroring type according to values of first 2 bits.
	switch (val & 3)
	{
	case 0:
		ntMirror_ = NESNameTableMirroringType::ONE_SCREEN; // @TODO: Should be one-screen, LOWER bank.
		break;

	case 1:
		ntMirror_ = NESNameTableMirroringType::ONE_SCREEN; // @TODO: Should be one-screen, UPPER bank.
		break;

	case 2:
		ntMirror_ = NESNameTableMirroringType::VERTICAL;
		break;

	case 3:
		ntMirror_ = NESNameTableMirroringType::HORIZONTAL;
		break;
	}

	// Change PRG and CHR Bank modes to val of bits 2-3 and bits 4 respectively. 
	prgBankMode_ = (val >> 2) & 3;
	chrBankMode_ = (val >> 4) & 1;
}


void NESMMC1::WriteCHRBankRegister(u8 bankNum, u8 val)
{
	assert(bankNum < 2);

	switch (chrBankMode_)
	{
	case 0: // One 8 KB Bank
		// Ignore bit 0 so we choose from 16 banks for bank 0.
		// Bank 1 will be the next 4KB bank of CHR afterwards.
		// @NOTE: NESMemCHRBank is 8KB, so we need to convert the index
		// to one that works with this type.
		chrBankIndices_[0] = (val & 0x1E) % (chr_.size() * 2);
		chrBankIndices_[1] = ((val & 0x1E) | 1) % (chr_.size() * 2);
		break;

	case 1: // Two 4 KB Banks
		chrBankIndices_[bankNum] = (val & 0x1F) % (chr_.size() * 2);
		break;
	}
}


void NESMMC1::WritePRGBankRegister(u8 val)
{
	// @TODO: Handle change of SRAM.
	switch (prgBankMode_)
	{
	case 0:
	case 1: // 32 KB Bank
		// Ignore bit 0 so we choose from 16 banks. (32 indices)
		prgBankIndices_[0] = (val & 0xE) % prg_.size();
		prgBankIndices_[1] = ((val & 0xE) | 1) % prg_.size();
		break;

	case 2: // Switch 16 KB Bank at $C000
		prgBankIndices_[1] = (val & 0xF) % prg_.size();
		break;

	case 3: // Switch 16 KB Bank at $8000
		prgBankIndices_[0] = (val & 0xF) % prg_.size();
		break;
	}
}


void NESMMC1::HandleRegisterWrite(u16 addr, u8 val)
{
	assert(addr >= 0x8000);

	if (NESHelper::IsBitSet(val, 7))
	{
		// Clear shift register to initial state.
		shiftReg_ = 0x10;
	}
	else
	{
		const u8 newShift = (shiftReg_ >> 1) | ((val & 1) << 4);

		if (NESHelper::IsBitSet(shiftReg_, 0))
		{
			// Shift register filled, write to correct internal register
			// depending on the write address and reset shift register.

			if (addr < 0xA000) // Control Register
				WriteControlRegister(newShift);
			else if (addr < 0xE000) // CHR Bank 0 or 1
				WriteCHRBankRegister((addr - 0xA000) / 0x2000, newShift);
			else // PRG Bank
				WritePRGBankRegister(newShift);

			shiftReg_ = 0x10;
		}
		else
			shiftReg_ = newShift;
	}
}


void NESMMC1::Write8(u16 addr, u8 val)
{
	if (addr < 0x2000) // CHR-ROM / CHR-RAM
	{
		const auto addrBankNum = addr / 0x1000;
		const auto chrIdx = chrBankIndices_[addrBankNum];

		if (chrBankMode_ == 0) // 8 KB Banks
			chr_[chrIdx].Write8(addr, val);
		else // 4+4 KB Banks - We consider upper or lower part of the 8KB NESMemCHRBank.
			chr_[chrIdx / 2].Write8(addr - ((chrIdx % 2) * 0x1000), val);
	}
	else if (addr >= 0x6000 && addr < 0x8000) // SRAM @TODO
		sram_[0].Write8(addr - 0x6000, val);
	else if (addr >= 0x8000) // MMC1 Registers
		HandleRegisterWrite(addr, val);
}


u8 NESMMC1::Read8(u16 addr) const
{
	if (addr < 0x2000) // CHR-ROM / CHR-RAM
	{
		const auto addrBankNum = addr / 0x1000;
		const auto chrIdx = chrBankIndices_[addrBankNum];

		if (chrBankMode_ == 0) // 8 KB Banks
			return chr_[chrIdx / 2].Read8(addr);
		else // 4+4 KB Banks - We consider upper or lower part of the 8KB NESMemCHRBank.
			return chr_[chrIdx / 2].Read8(addr - ((chrIdx % 2) * 0x1000));
	}
	else if (addr >= 0x6000 && addr < 0x8000) // SRAM @TODO
		return sram_[0].Read8(addr - 0x6000);
	else if (addr >= 0x8000) // PRG-ROM Banks
		return prg_[prgBankIndices_[(addr - 0x8000) / 0x4000]].Read8(addr & 0x3FFF);
	else
		return 0;
}