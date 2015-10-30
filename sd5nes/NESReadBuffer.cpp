#include "NESReadBuffer.h"


NESReadBuffer::NESReadBuffer(const std::vector<u8>& romFileData) :
romFileData_(romFileData),
romFileReadPos_(0)
{
}


NESReadBuffer::~NESReadBuffer()
{
}


bool NESReadBuffer::ReadNext8(u8* outVal)
{
	if (romFileReadPos_ >= romFileData_.size())
		return false; // End of data.

	if (outVal != nullptr)
		*outVal = romFileData_[romFileReadPos_];

	++romFileReadPos_;
	return true;
}


bool NESReadBuffer::ReadNext(size_t readSize, std::vector<u8>* outVec)
{
	std::vector<u8> vec;
	u8 currentVal;

	for (size_t i = 1; i <= readSize; ++i)
	{
		if (!ReadNext8(&currentVal))
			return false;

		vec.emplace_back(currentVal);
	}

	if (outVec != nullptr)
		*outVec = vec;

	return true;
}


bool NESReadBuffer::ReadNextStr(size_t readSize, std::string* outStr)
{
	std::vector<u8> vec;
	if (!ReadNext(readSize, &vec))
		return false;

	// Convert to string.
	if (outStr != nullptr)
		*outStr = std::string(vec.begin(), vec.end());

	return true;
}