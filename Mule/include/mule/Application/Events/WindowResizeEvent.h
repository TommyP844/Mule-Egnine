#pragma once

#include "Event.h"

namespace Mule
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height) 
			: 
			Event(EventType::WindowResize),
			Width(width),
			Height(height)
		{}

		const uint32_t Width;
		const uint32_t Height;
	};
}