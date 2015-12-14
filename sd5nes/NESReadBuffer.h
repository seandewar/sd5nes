#pragma once

#include <vector>
#include <stdexcept>

#include "NESTypes.h"

/**
* Errors relating to read errors from the NESReadBuffer class.
*/
class NESReadBufferException : public std::runtime_error
{
public:
	explicit NESReadBufferException(const char* msg) :
		std::runtime_error(msg)
	{ }

	explicit NESReadBufferException(const std::string& msg) :
		std::runtime_error(msg)
	{ }

	virtual ~NESReadBufferException()
	{ }
};

/**
* Contains some methods for reading from a buffer.
*/
class NESReadBuffer
{
public:
	NESReadBuffer(const std::vector<u8>& romFileData);
	~NESReadBuffer();

	// Reads the next 8 bits from ROM file buffer.
	u8 ReadNext8();

	// Reads the next X amount of bytes from ROM file buffer.
	std::vector<u8> ReadNext(std::size_t readSize);

	// Reads the next X amount of bytes from ROM file buffer and converts to string.
	std::string ReadNextStr(std::size_t readSize);

private:
	const std::vector<u8>& romFileData_;
	std::size_t romFileReadPos_;
};

