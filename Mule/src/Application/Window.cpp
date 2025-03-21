#include "Application/Window.h"

#include "spdlog/spdlog.h"

// Events
#include "Application/Events/WindowResizeEvent.h"
#include "Application/Events/DropFileEvent.h"


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

	const std::vector<Ref<Event>> Window::PollEvents()
	{
		mEvents.clear();
		glfwPollEvents();
		return mEvents;
	}

	void Window::PushEvent(Ref<Event> event)
	{
		mEvents.push_back(event);
	}

	const glm::vec2& Window::GetMousePos()
	{
		return mMousePos;
	}

	void Window::SetMousePos(const glm::vec2& pos)
	{
		glfwSetCursorPos(mWindow, pos.x, pos.y);
		mMousePos = pos;
	}

	bool Window::IsMouseButtonDown(MouseButton button)
	{
		return mMouseButtonStates[(uint32_t)button];
	}

	bool Window::IsKeyDown(KeyCode key)
	{
		return mKeyStates[(uint32_t)key];
	}

	void Window::SetupEventCallbacks()
	{
		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
			Window* currWindow = (Window*)glfwGetWindowUserPointer(window);
			Ref<WindowResizeEvent> event = MakeRef<WindowResizeEvent>(width, height);
			currWindow->PushEvent(event);
			});


		glfwSetDropCallback(mWindow, [](GLFWwindow* window, int pathCount, const char** paths) {
			Window* currWindow = (Window*)glfwGetWindowUserPointer(window);
			Ref<DropFileEvent> event = MakeRef<DropFileEvent>(pathCount, paths);
			currWindow->PushEvent(event);
			});

		
	}
}