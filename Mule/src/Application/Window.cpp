#include "Application/Window.h"

#include "spdlog/spdlog.h"

// Events
#include "Application/Events/WindowResizeEvent.h"


namespace Mule
{
	Window::Window(std::string_view title)
		:
		mTitle(title),
		mWidth(1920),
		mHeight(1080)
	{
		if (!glfwInit())
		{
			SPDLOG_CRITICAL("Failed to init GLFW");
			exit(1);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		mWindow = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);

		if (!mWindow)
		{
			SPDLOG_CRITICAL("Failed to create window: {}", title);
			exit(1);
		}
		
		glfwSetWindowUserPointer(mWindow, this);
		SetupEventCallbacks();
		glfwMakeContextCurrent(mWindow);
	}

	Window::~Window()
	{
	}

	bool Window::WindowOpen() const
	{
		return !glfwWindowShouldClose(mWindow);
	}

	void Window::PollEvents()
	{
		mEvents.clear();
		glfwPollEvents();
	}

	void Window::PushEvent(Ref<Event> event)
	{
		mEvents.push_back(event);
	}

	void Window::SetupEventCallbacks()
	{
		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
			Window* currWindow = (Window*)glfwGetWindowUserPointer(window);
			Ref<WindowResizeEvent> event = MakeRef<WindowResizeEvent>(width, height);
			currWindow->PushEvent(event);
			});
	}
}