#pragma once

#include "Application/MouseButton.h"
#include "Event.h"

namespace Mule
{
	class MouseButtonEvent : public Event
	{
	public:
		MouseButtonEvent(MouseButton button, bool pressed, KeyCode modifer)
			:
			Event(EventType::MouseButtonEvent),
			mPressed(pressed),
			mButton(button),
			mModifier(modifer)
		{ }

		MouseButton GetMouseButton() const { return mButton; }
		bool IsButtonPressed() const { return mPressed; }
		KeyCode GetModifier() const { return mModifier; }
	private:
		bool mPressed;
		MouseButton mButton;
		KeyCode mModifier;
	};
}