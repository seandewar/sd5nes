#include "NESReadBuffer.h"


NESReadBuffer::NESReadBuffer(const std::vector<u8>& romFileData) :
romFileData_(romFileData),
romFileReadPos_(0)
{
}


NESReadBuffer::~NESReadBuffer()
{
}


u8 NESReadBuffer::ReadNext8()
{
	if (romFileReadPos_ >= romFileData_.size())
		throw NESReadBufferException("Reached end of data on read!"); // End of data.

	return romFileData_[romFileReadPos_++];
}


std::vector<u8> NESReadBuffer::ReadNext(std::size_t readSize)
{
	std::vector<u8> vec;
	for (std::size_t i = 0; i < readSize; ++i)
		vec.emplace_back(ReadNext8());

	return vec;
}


std::string NESReadBuffer::ReadNextStr(std::size_t readSize)
{
	const auto vec = ReadNext(readSize);
	return std::string(vec.begin(), vec.end());
}