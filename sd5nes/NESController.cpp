#include "NESController.h"

#include <cassert>

#include "NESHelper.h"


NESStandardController::NESStandardController() :
allowUpDownOrLeftRight_(false),
isStrobeHigh_(false)
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
	if (isStrobeHigh_)
		buttonNumber_ = 0;
}


u8 NESStandardController::ReadController()
{
	return 0;
}