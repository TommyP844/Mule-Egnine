#pragma once

#include <GLFW/glfw3.h>
#include <string>

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

	private:
		GLFWwindow* mWindow;
		std::string mTitle;
		int mWidth, mHeight;
	};
}