#include "NESHelper.h"

#include <random>
#include <chrono>


bool NESHelper::GetRandomBool(double trueChance)
{
	// Seed the random engine with the current time as seed.
	static std::mt19937 engine(static_cast<unsigned int>(
		std::chrono::system_clock::now().time_since_epoch().count()
		)
	);

	std::bernoulli_distribution d(trueChance);
	return d(engine);
}