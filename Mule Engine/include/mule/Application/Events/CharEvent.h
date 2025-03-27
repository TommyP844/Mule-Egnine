#pragma once

#include "Application/KeyCode.h"

#include "Event.h"

namespace Mule
{
	class CharEvent : public Event
	{
	public:
		CharEvent(uint32_t c)
			:
			Event(EventType::CharEvent),
			mChar(c)
		{ }

		uint32_t GetChar() const { return mChar; }

	private:
		uint32_t mChar;
	};
}