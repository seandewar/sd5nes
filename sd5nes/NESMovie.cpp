#include "NESMovie.h"

#include <fstream>
#include <sstream>

#include "NESReadBuffer.h"


NESMovie::NESMovie()
{
	ResetLoadedState();
}


NESMovie::~NESMovie()
{
}


void NESMovie::ResetLoadedState()
{
	isMovieLoaded_ = false;

	movieFileName_.clear();
}


void NESMovie::AddKeyValue(const std::string& keyName, const std::string& keyVal)
{
	if (std::find(movieKnownIntKeys.begin(), movieKnownIntKeys.end(), keyName) != movieKnownIntKeys.end())
	{
		// We have a known integer key value!
		// Parse keyVal as integer.
		headerKeyVals_.emplace(keyName, std::make_unique<NESMovieHeaderValueInteger>(std::stol(keyVal)));
	}
	else
	{
		// We either have a known string key value or we
		// have some unknown value type - assume string anyway...
		headerKeyVals_.emplace(keyName, std::make_unique<NESMovieHeaderValueString>(keyVal));
	}
}


void NESMovie::ParseMovieFileData(const std::vector<u8>& data)
{
	// @TODO: Support other movie file formats other than FM2?
	NESReadBuffer buf(data);

	// Parse movie header first
	try
	{
		while (buf.ReadNext8() != '|')
		{
			u8 readChar;

			// Read the name of the key.
			std::ostringstream keyNameOss;
			while (readChar = buf.ReadNext8() != ' ')
				keyNameOss << readChar;

			// Read key value.
			std::ostringstream keyValOss;
			while (readChar = buf.ReadNext8() != '\n')
			{
				// This should be part of a \n, so just ignore it if it's present.
				if (readChar == '\r')
					continue;

				keyValOss << readChar;
			}

			AddKeyValue(keyNameOss.str(), keyValOss.str());
		}
	}
	catch (const NESReadBufferException&)
	{
		throw NESMovieLoadException("Failed to parse movie header!");
	}

	// Check if we support the current header settings being used...
	// @TODO: Add more support for controllers and stuff...
	const auto fourscore = GetKeyIntValue("fourscore");
	const auto port0 = GetKeyIntValue("port0");
	const auto port1 = GetKeyIntValue("port1");

	if (fourscore != nullptr && fourscore->GetValueAsBool())
		throw NESMovieLoadException("Fourscore not supported for movie playback.");
	else if ((port0 != nullptr && port0->GetValue() == 2) ||
		(port1 != nullptr && port1->GetValue() == 2))
		throw NESMovieLoadException("Zapper controller not supported for movie playback.");

	// Parse input log
	try
	{
		const auto binary = GetKeyIntValue("binary");
		if (binary != nullptr && binary->GetValueAsBool())
		{
			// Binary mode
			// @TODO
		}
		else
		{
			// Text mode
			// @TODO: Support this movie format first - this is the most used one!
		}
	}
	catch (const NESReadBufferException&)
	{
		throw NESMovieLoadException("Failed to parse movie input log!");
	}
}


std::vector<u8> NESMovie::ReadMovieFile(const std::string& fileName)
{
	std::ifstream fileStream(fileName, std::ios_base::in | std::ios_base::binary);
	std::vector<u8> fileData;

	// Read movie file.
	while (fileStream.good())
		fileData.emplace_back(fileStream.get());

	// Check if end-of-file (successful read).
	if (fileStream.eof())
		return fileData;

	// If not EOF, then read failed.
	throw NESMovieLoadException("Failed to read NES Movie file!");
}


void NESMovie::LoadMovie(const std::string& fileName)
{
	ResetLoadedState();

	movieFileName_ = fileName;
	ParseMovieFileData(ReadMovieFile(fileName));

	isMovieLoaded_ = true;
}