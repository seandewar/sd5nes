#pragma once

#include <string>
#include <unordered_map>
#include <array>
#include <memory>

#include "NESTypes.h"
#include "NESException.h"

/**
* Exception class thrown when a movie file cannot be loaded.
*/
class NESMovieLoadException : public NESException
{
public:
	explicit NESMovieLoadException(const std::string& msg) : NESException(msg) { }
	explicit NESMovieLoadException(const char* msg) : NESException(msg) { }
};

/**
* Type of the movie file header keyvalue's data.
*/
enum class NESMovieHeaderValueType
{
	STRING,
	INTEGER
};

/**
* Struct for holding the different key values of the movie file header info.
*/
class NESMovieHeaderValueBase
{
public:
	NESMovieHeaderValueBase() { }
	virtual ~NESMovieHeaderValueBase() { }

	virtual NESMovieHeaderValueType GetType() const = 0;
};

/**
* Represents a movie header string key value.
*/
class NESMovieHeaderValueString : public NESMovieHeaderValueBase
{
public:
	explicit NESMovieHeaderValueString(const std::string& val) : val_(val) { }
	virtual ~NESMovieHeaderValueString() { }

	inline NESMovieHeaderValueType GetType() const { return NESMovieHeaderValueType::STRING; }

	inline std::string GetValue() const { return val_; }

private:
	const std::string val_;
};

/**
* Represents a movie header integer key value. Can also hold boolean values.
*/
class NESMovieHeaderValueInteger : public NESMovieHeaderValueBase
{
public:
	explicit NESMovieHeaderValueInteger(s32 val) : val_(val) { }
	explicit NESMovieHeaderValueInteger(bool val) : val_((val ? 1 : 0)) { }
	virtual ~NESMovieHeaderValueInteger() { }

	inline NESMovieHeaderValueType GetType() const { return NESMovieHeaderValueType::INTEGER; }

	inline s32 GetValue() const { return val_; }
	inline bool GetValueAsBool() const { return (val_ != 0 ? true : false); }

private:
	const s32 val_;
};

/**
* Array containing the known key names used by the parser for integer keys.
*/
const std::array<std::string, 11> movieKnownIntKeys = {
	{
		"version",
		"emuVersion",
		"rerecordCount",
		"palFlag",
		//"NewPPU", // Used by FCEUX
		"FDS",
		"fourscore",
		"port0",
		"port1",
		"port2",
		"binary",
		"length"
	}
};

/**
* Array containing the known key names used by the parser for string keys.
*/
const std::array<std::string, 6> movieKnownStrKeys = {
	{
		"romFilename",
		"comment",
		"subtitle",
		"guid",
		"romChecksum",
		"savestate"
	}
};

/**
* Allows playback for NES movie files.
*/
class NESMovie
{
public:
	NESMovie();
	~NESMovie();

	/**
	* Loads the specified movie file.
	*/
	void LoadMovie(const std::string& fileName);

	/**
	* Gets a key value from its key name.
	* Returns nullptr if key not found.
	*/
	template<typename T>
	const T* GetKeyValue(const std::string& keyName) const
	{
		const auto it = headerKeyVals_.find(keyName);
		if (it == headerKeyVals_.end())
			return nullptr;
		else
			return static_cast<T*>(it->second.get());
	}

	inline const NESMovieHeaderValueInteger* GetKeyIntValue(const std::string& keyName) const
	{
		return GetKeyValue<NESMovieHeaderValueInteger>(keyName);
	}

	inline const NESMovieHeaderValueString* GetKeyStrValue(const std::string& keyName) const
	{
		return GetKeyValue<NESMovieHeaderValueString>(keyName);
	}

private:
	bool isMovieLoaded_;
	std::string movieFileName_;

	/* Stored header key value info. */
	std::unordered_map<std::string, std::unique_ptr<NESMovieHeaderValueBase>> headerKeyVals_;

	void ResetLoadedState();

	/**
	* Adds a key value using its parsed string data.
	*/
	void AddKeyValue(const std::string& keyName, const std::string& keyVal);

	/**
	* Parses the movie file data to be usable by the emulator.
	*/
	void ParseMovieFileData(const std::vector<u8>& data);

	/**
	* Reads a movie file into memory and returns the binary contents.
	*/
	std::vector<u8> ReadMovieFile(const std::string& fileName);
};

