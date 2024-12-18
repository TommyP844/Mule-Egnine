#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace Engine
{
	class Window
	{
	public:
		Window(std::string_view title);
		~Window();
		Window(const Window&) = delete;

		GLFWwindow* GetGLFWWindow() const { return mWindow; }
		std::string_view GetTitle() const { return mTitle; }

	private:
		GLFWwindow* mWindow;
		std::string mTitle;
	};
}