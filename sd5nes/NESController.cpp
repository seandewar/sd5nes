#include "NESController.h"

#include <cassert>

#include "NESHelper.h"


/* The number of the first unknown button. */
#define NES_UNKNOWN_BUTTON_NUMBER 8


NESControllerButton NESStandardController::GetButtonFromNumber(unsigned int buttonNumber)
{
	switch (buttonNumber)
	{
	case 0:
		return NESControllerButton::A;

	case 1:
		return NESControllerButton::B;

	case 2:
		return NESControllerButton::SELECT;

	case 3:
		return NESControllerButton::START;

	case 4:
		return NESControllerButton::UP;

	case 5:
		return NESControllerButton::DOWN;

	case 6:
		return NESControllerButton::LEFT;

	case 7:
		return NESControllerButton::RIGHT;

	default:
		return NESControllerButton::UNKNOWN;
	}
}


NESStandardController::NESStandardController() :
allowUpDownOrLeftRight_(false),
isStrobeHigh_(false),
buttonNumber_(0)
{
	// Init button state map.
	buttonStates_.emplace(NESControllerButton::A, false);
	buttonStates_.emplace(NESControllerButton::B, false);
	buttonStates_.emplace(NESControllerButton::SELECT, false);
	buttonStates_.emplace(NESControllerButton::START, false);
	buttonStates_.emplace(NESControllerButton::UP, false);
	buttonStates_.emplace(NESControllerButton::DOWN, false);
	buttonStates_.emplace(NESControllerButton::LEFT, false);
	buttonStates_.emplace(NESControllerButton::RIGHT, false);
}


NESStandardController::~NESStandardController()
{
}


bool NESStandardController::GetButtonState(NESControllerButton button) const
{
	const auto it = buttonStates_.find(button);
	assert(it != buttonStates_.end());

	return it->second;
}


void NESStandardController::SetButtonStateInternal(NESControllerButton button, bool isPressed)
{
	const auto it = buttonStates_.find(button);
	assert(it != buttonStates_.end());

	it->second = isPressed;
}


void NESStandardController::SetButtonState(NESControllerButton button, bool isPressed)
{
	if (!allowUpDownOrLeftRight_ && isPressed)
	{
		// Unpress the opposite direction button.
		switch (button)
		{
		case NESControllerButton::UP:
			SetButtonStateInternal(NESControllerButton::DOWN, false);
			break;

		case NESControllerButton::DOWN:
			SetButtonStateInternal(NESControllerButton::UP, false);
			break;

		case NESControllerButton::LEFT:
			SetButtonStateInternal(NESControllerButton::RIGHT, false);
			break;

		case NESControllerButton::RIGHT:
			SetButtonStateInternal(NESControllerButton::LEFT, false);
			break;
		}
	}

	SetButtonStateInternal(button, isPressed);
}


void NESStandardController::WriteController(u8 val)
{
	isStrobeHigh_ = NESHelper::IsBitSet(val, 0);

	// Check status of strobe.
	if (isStrobeHigh_)
		buttonNumber_ = 0;
}


u8 NESStandardController::ReadController()
{
	u8 returnVal = 0;

	// If we have already cycled through all buttons, set serial controller bit (#0).
	// @NOTE: Authentic standard NES controllers do this.
	if (buttonNumber_ >= NES_UNKNOWN_BUTTON_NUMBER)
	{
		// Sanity check to make sure this doesn't ever overflow.
		buttonNumber_ = NES_UNKNOWN_BUTTON_NUMBER;
		NESHelper::SetRefBit(returnVal, 0);
	}
	else
	{
		// Set serial controller bit if button is pressed, otherwise clear it.
		if (GetButtonState(GetButtonFromNumber(buttonNumber_)))
			NESHelper::SetRefBit(returnVal, 0);
		else
			NESHelper::ClearRefBit(returnVal, 0);

		// Check strobe status.
		if (isStrobeHigh_)
			buttonNumber_ = 0;
		else
			++buttonNumber_;
	}

	return returnVal;
}