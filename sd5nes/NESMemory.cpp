#include "NESMemory.h"


NESMemory::NESMemory(uleast16 size) :
data_(size)
{
	ZeroMemory();
}


NESMemory::NESMemory(const std::vector<u8>& data) :
data_(data)
{
}


NESMemory::~NESMemory()
{
}


void NESMemory::ZeroMemory()
{
	std::fill(data_.begin(), data_.end(), 0);
}


bool NESMemory::Write8(u16 addr, u8 val)
{
	if (addr >= data_.size())
		return false;

	data_[addr] = val;
	return true;
}


bool NESMemory::Read8(u16 addr, u8* outVal) const
{
	if (addr >= data_.size())
		return false;

	if (outVal != nullptr)
		*outVal = data_[addr];

	return true;
}


bool NESMemory::Read16(u16 addr, u16* outVal) const
{
	u8 low, hi;

	// Pull least-significant byte first.
	if (!Read8(addr, &low))
		return false;

	if (!Read8(addr + 1, &hi))
		return false;

	// Convert to 16-bit val.
	if (outVal != nullptr)
		*outVal = ((hi << 8) | low);

	return true;
}


uleast32 NESMemory::GetSize() const
{
	return data_.size();
}