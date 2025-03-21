#pragma once

#include "Ref.h"
#include "Events/Event.h"
#include "KeyCode.h"
#include "MouseButton.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>
#include <functional>

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

		const std::vector<Ref<Event>> PollEvents();
		void PushEvent(Ref<Event> event);

		const glm::vec2& GetMousePos();
		void SetMousePos(const glm::vec2& pos);
		bool IsMouseButtonDown(MouseButton button);

		bool IsKeyDown(KeyCode key);

	private:
		GLFWwindow* mWindow;
		std::string mTitle;
		int mWidth, mHeight;

		glm::vec2 mMousePos;
		bool mKeyStates[512];
		bool mMouseButtonStates[3];

		std::vector<Ref<Event>> mEvents;

		void SetupEventCallbacks();
	};
}