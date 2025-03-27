#pragma once

#include "Application/KeyCode.h"
#include "Event.h"

namespace Mule
{
	class KeyboardEvent : public Event
	{
	public:
		KeyboardEvent(KeyCode key, bool pressed, KeyCode modifier)
			:
			Event(EventType::KeyboardEvent),
			mKey(key),
			mModifier(modifier),
			mPressed(pressed)
		{ }

		KeyCode GetKey() const { return mKey; }
		KeyCode GetModifier() const { return mModifier; }
		bool IsKeyPressed() const { return mPressed; }

	private:
		KeyCode mKey, mModifier;
		bool mPressed;
	};
}