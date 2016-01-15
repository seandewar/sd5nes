#pragma once

#include <stdexcept>

/**
* Base exception class for exceptions thrown by the emulator.
*/
class NESException : public std::runtime_error
{
public:
	explicit NESException(const std::string& msg) : std::runtime_error(msg) { }
	explicit NESException(const char* msg) : std::runtime_error(msg) { }
	virtual ~NESException() { }
};