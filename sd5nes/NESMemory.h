#pragma once

#include <array>
#include <vector>
#include <stdexcept>
#include <cassert>

#include "NESTypes.h"
#include "NESMemoryConstants.h"

/**
* Exception thrown during memory access errors.
*/
class NESMemoryException : public std::runtime_error
{
public:
	explicit NESMemoryException(const char* msg) :
		std::runtime_error(msg)
	{ }

	explicit NESMemoryException(const std::string& msg) :
		std::runtime_error(msg)
	{ }

	virtual ~NESMemoryException()
	{ }
};

/**
* Interface that declares some basic read/write functions.
*/
class INESMemoryInterface
{
public:
	virtual ~INESMemoryInterface() { }

	virtual void Write8(u16 addr, u8 val) = 0;
	virtual u8 Read8(u16 addr) const = 0;
};

/**
* Represents the memory used by a hardware component of the NES system.
*/
template <u16 size>
class NESMemory : public INESMemoryInterface
{
public:
	NESMemory() { ZeroMemory(); }
	explicit NESMemory(const std::vector<u8>& vec) { CopyFromVector(vec); }
	virtual ~NESMemory() { }

	/**
	* Sets all the allocated memory to zero.
	*/
	inline void ZeroMemory() { std::fill(data_.begin(), data_.end(), 0); }

	/**
	* Copies the contents from a vector into zero'd memory.
	*/
	void CopyFromVector(const std::vector<u8>& vec)
	{
		// Copying from a buffer which is larger than ours
		// is probably bad...
		assert(vec.size() <= data_.size());

		for (u16 i = 0; i < data_.size(); ++i)
		{
			// Fill with data from buffer. If we have reached the end
			// of the buffer, fill the rest with zeros.
			data_[i] = (i < vec.size() ? vec[i] : 0);
		}
	}

	/**
	* Writes 8-bits to the memory at a specified location with the specified value.
	*/
	void Write8(u16 addr, u8 val) override
	{
		if (addr >= data_.size())
			throw NESMemoryException("Cannot write to memory outside of allocated space!");

		data_[addr] = val;
	}

	/**
	* Reads 8-bits from the memory at a specified location.
	*/
	u8 Read8(u16 addr) const override
	{
		if (addr >= data_.size())
			throw NESMemoryException("Cannot read from memory outside of allocated space!");

		return data_[addr];
	}

	/**
	* Gets the allocated size.
	*/
	inline u32 GetSize() const { return data_.size(); }

private:
	std::array<u8, size> data_;
};

/**
* Base class for memory mappers.
*/
class NESMemoryMapper : public INESMemoryInterface
{
public:
	NESMemoryMapper();
	virtual ~NESMemoryMapper();

	/**
	* Writes 8-bits to the memory at a specified mapped location with the specified value.
	* Uses protected GetMapping() to get the correct mapping.
	*/
	virtual void Write8(u16 addr, u8 val) override;

	/**
	* Reads 8-bits from the memory at a specified mapped location with the specified value.
	* Uses protected GetMapping() to get the correct mapping.
	*/
	virtual u8 Read8(u16 addr) const override;

protected:
	/**
	* Gets the NESMemory and mapped address corrisponding to an unmapped address.
	*/
	virtual std::pair<INESMemoryInterface*, u16> GetMapping(u16 addr) const = 0;
};
