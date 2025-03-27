#pragma once

#include "Event.h"

namespace Mule
{
	class MouseMoveEvent : public Event
	{
	public:
		MouseMoveEvent(float x, float y)
			:
			Event(EventType::MouseMoveEvent),
			mX(x),
			mY(y)
		{ }

		float GetX() const { return mX; }
		float GetY() const { return mY; }

	private:
		float mX, mY;
	};
}
