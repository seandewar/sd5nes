#pragma once

#include <vector>

#include "NESTypes.h"

/**
* Contains some methods for reading from a buffer.
*/
class NESReadBuffer
{
public:
	NESReadBuffer(const std::vector<u8>& romFileData);
	~NESReadBuffer();

	// Reads the next 8 bits from ROM file buffer.
	bool ReadNext8(u8* outVal);

	// Reads the next X amount of bytes from ROM file buffer.
	bool ReadNext(size_t readSize, std::vector<u8>* outVec);

	// Reads the next X amount of bytes from ROM file buffer and converts to string.
	bool ReadNextStr(size_t readSize, std::string* outStr);

private:
	const std::vector<u8>& romFileData_;
	size_t romFileReadPos_;
};

