#pragma once

#include <typeinfo> // for uint32_t

namespace Mule
{
	enum class EventType
	{
		WindowResizeEvent,
		DropFileEvent,
		MouseButtonEvent,
		KeyboardEvent,
		MouseMoveEvent,
		CharEvent
	};


	class Event
	{
	public:
		Event(EventType type) : Type(type), mHandled(false) {}

		const EventType Type;

		bool IsHandled() const { return mHandled; }
		void SetHandled() { mHandled = true; }

	private:
		bool mHandled = false;
	};
}
