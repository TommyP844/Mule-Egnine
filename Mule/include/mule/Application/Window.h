#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <functional>

#include "Ref.h"
#include "Events/Event.h"

namespace Mule
{
	class Window
	{
	public:
		Window(std::string_view title);
		~Window();
		Window(const Window&) = delete;

		GLFWwindow* GetGLFWWindow() const { return mWindow; }
		const std::string GetTitle() const { return mTitle; }

		int GetWidth() const { return mWidth; }
		int GetHeight() const { return mHeight; }

		bool WindowOpen() const;

		void PollEvents();
		void PushEvent(Ref<Event> event);
		const std::vector<Ref<Event>> GetFrameEvents() const { return mEvents; }


	private:
		GLFWwindow* mWindow;
		std::string mTitle;
		int mWidth, mHeight;

		std::vector<Ref<Event>> mEvents;

		void SetupEventCallbacks();
	};
}