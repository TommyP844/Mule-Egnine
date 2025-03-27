#include "Application/Window.h"

#include "spdlog/spdlog.h"

// Events
#include "Application/Events/WindowResizeEvent.h"
#include "Application/Events/DropFileEvent.h"
#include "Application/Events/KeyboardEvent.h"
#include "Application/Events/MouseButtonEvent.h"
#include "Application/Events/MouseMoveEvent.h"
#include "Application/Events/CharEvent.h"


namespace Mule
{
	Window::Window(std::string_view title)
		:
		mTitle(title),
		mWidth(1920),
		mHeight(1080),
		mKeyStates({0}),
		mMouseButtonStates({0})
	{
		if (!glfwInit())
		{
			SPDLOG_CRITICAL("Failed to init GLFW");
			exit(1);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
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

	const std::vector<Ref<Event>>& Window::PollEvents()
	{
		mEvents.clear();
		glfwPollEvents();
		return mEvents;
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
			currWindow->mEvents.push_back(event);
			});

		glfwSetDropCallback(mWindow, [](GLFWwindow* window, int pathCount, const char** paths) {
			Window* currWindow = (Window*)glfwGetWindowUserPointer(window);
			Ref<DropFileEvent> event = MakeRef<DropFileEvent>(pathCount, paths);
			currWindow->mEvents.push_back(event);
			});

		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
			Window* currWindow = (Window*)glfwGetWindowUserPointer(window);
			Ref<MouseMoveEvent> event = MakeRef<MouseButtonEvent>((MouseButton)button, (bool)action, (KeyCode)mods);
			currWindow->mEvents.push_back(event);

			currWindow->mMouseButtonStates[button] = (bool)action;
			});

		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double x, double y) {
			Window* currWindow = (Window*)glfwGetWindowUserPointer(window);
			Ref<MouseMoveEvent> event = MakeRef<MouseMoveEvent>((float)x, (float)y);
			currWindow->mEvents.push_back(event);

			currWindow->mMousePos = { x, y };
			});

		glfwSetCharCallback(mWindow, [](GLFWwindow* window, uint32_t codepoint) {
			Window* currWindow = (Window*)glfwGetWindowUserPointer(window);
			Ref<CharEvent> event = MakeRef<CharEvent>(codepoint);
			currWindow->mEvents.push_back(event);
			});
		
		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			Window* currWindow = (Window*)glfwGetWindowUserPointer(window);
			Ref<KeyboardEvent> event = MakeRef<KeyboardEvent>((KeyCode)key, (bool)action, (KeyCode)mods);
			currWindow->mEvents.push_back(event);

			currWindow->mKeyStates[key] = (bool)action;
			});		
	}
}