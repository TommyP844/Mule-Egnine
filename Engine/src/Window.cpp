#include "Window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


namespace Engine
{
	Window::Window(std::string_view title)
		:
		mTitle(title)
	{
		if (!glfwInit())
		{

		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		mWindow = glfwCreateWindow(800, 600, mTitle.c_str(), nullptr, nullptr);
	}

	Window::~Window()
	{
	}
}