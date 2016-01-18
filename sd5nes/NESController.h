#pragma once

#include <unordered_map>

#include "NESTypes.h"

/**
* Allows controller input for the system.
*/
class INESController
{
public:
	INESController() { }
	virtual ~INESController() { }

	virtual void WriteController(u8 val) = 0;
	virtual u8 ReadController() = 0;
};

/**
* The different types of supported controller buttons on the NES.
*/
enum class NESControllerButton
{
	A,
	B,
	SELECT,
	START,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	UNKNOWN
};

/**
* Represents the standard controller used by the NES.
*/
class NESStandardController : public INESController
{
public:
	NESStandardController();
	virtual ~NESStandardController();

	/**
	* Releases all buttons so that they are not being pressed.
	*/
	virtual void ResetButtonStates();

	/**
	* Sets whether or not a controller button is currently being pressed.
	*/
	virtual void SetButtonState(NESControllerButton button, bool isPressed);

	/**
	* Gets whether or not a controller button is currently being pressed.
	*/
	virtual bool GetButtonState(NESControllerButton button) const;
	
	virtual void WriteController(u8 val) override;
	virtual u8 ReadController() override;

	/**
	* Sets whether or not pressing Up+Down / Left+Right simultaneously should be allowed.
	*/
	inline void SetUpDownOrLeftRightAllowed(bool isAllowed) { allowUpDownOrLeftRight_ = isAllowed; }

	/**
	* Whether or not pressing Up+Down / Left+Right simultaneously is currently allowed.
	*/
	inline bool IsUpDownOrLeftRightAllowed() const { return allowUpDownOrLeftRight_; }

private:
	/**
	* Gets the corrisponding NESControllerButton from its number.
	*/
	static NESControllerButton GetButtonFromNumber(unsigned int buttonNumber);

	std::unordered_map<NESControllerButton, bool> buttonStates_;

	// Whether or not pressing Up+Down / Left+Right simultaneously should be allowed.
	bool allowUpDownOrLeftRight_;

	bool isStrobeHigh_;
	unsigned int buttonNumber_;

	void SetButtonStateInternal(NESControllerButton button, bool isPressed);
};