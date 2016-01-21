#pragma once

#include "NESTypes.h"

/**
* Base struct for the 5 different sound channels of the APU.
*/
struct NESAPUChannel
{
	bool isEnabled;
	u8 timer;
	u8 lengthCounter;
};

/**
* Struct for the 2 different pulse channels of the APU.
*/
struct NESAPUPulseChannel : public NESAPUChannel
{
	u8 envelope;
	u8 sweep;
};

/**
* Struct for the triangle channel of the APU.
*/
struct NESAPUTriangleChannel : public NESAPUChannel
{
	u8 linearCounter;
};

/**
* Struct for the noise channel of the APU.
*/
struct NESAPUNoiseChannel : public NESAPUChannel
{
	u8 envelope;
	u8 linearFeedbackReg;
};

/**
* Struct for the DMC channel of the APU.
*/
struct NESAPUDMCChannel : public NESAPUChannel
{
	// @TODO.
};

/**
* The different register types of the APU.
*/
enum class NESAPURegisterType
{
	/* Pulse 1 Channel */
	PULSE1_0,
	PULSE1_1,
	PULSE1_2,
	PULSE1_3,

	/* Pulse 2 Channel */
	PULSE2_0,
	PULSE2_1,
	PULSE2_2,
	PULSE2_3,

	/* Triangle Channel */
	TRIANGLE_0,
	TRIANGLE_1,
	TRIANGLE_2,

	/* Noise Channel */
	NOISE_0,
	NOISE_1,
	NOISE_2,

	/* DMC Channel */
	DMC_0,
	DMC_1,
	DMC_2,
	DMC_3,

	/* Other Registers */
	STATUS_CONTROL,
	FRAME_COUNTER
};

/**
* Represents the APU that was responsible for the generation of sounds
* and music. It was implemented inside of the CPU on the NES.
*/
class NESAPU
{
public:
	NESAPU();
	~NESAPU();

	u8 ReadRegister(NESAPURegisterType type);
	void WriteRegister(NESAPURegisterType type, u8 val);

	/**
	* Ticks the APU for one cycle.
	*/
	void Tick();

private:
	u8 regStatus_;

	NESAPUPulseChannel pulse1_;
	NESAPUPulseChannel pulse2_;
	NESAPUTriangleChannel triangle_;
	NESAPUNoiseChannel noise_;
	NESAPUDMCChannel dmc_;
};

