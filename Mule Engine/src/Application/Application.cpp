
#include "Application/Application.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "Graphics/API/GraphicsContext.h"

#include "imgui.h"

// Events
#include "Application/Events/Event.h"
#include "Application/Events/WindowResizeEvent.h"
#include "Graphics/Renderer/Renderer.h"

// STD
#include <vector>
#include <chrono>

namespace Mule
{
	Application::Application()
		:
		mRunning(true),
		mMinimized(false)
	{
#ifdef _DEBUG
		spdlog::set_pattern("[%H:%M:%S %z] [%n] [thread %t] %v");
		
		SPDLOG_INFO("Application Started");
#endif
		EngineContextDescription engineDescription{};
		engineDescription.WindowName = "Mule";
		engineDescription.ProjectPath = "C:/Development/Mule Projects/Test Project";

		mEngineContext = MakeRef<EngineContext>(engineDescription);
	}

	Application::~Application()
	{
		while (!mLayerStack.empty())
			mLayerStack.PopLayer();
		mEngineContext = nullptr;
	}

	void Application::Run()
	{
		WeakRef<Window> window = mEngineContext->GetWindow();
		WeakRef<ImGuiContext> imguiContext = mEngineContext->GetImGuiContext();

		float dt = 1.f;

		while (mRunning)
		{
			auto start = std::chrono::high_resolution_clock::now();

			std::vector<Ref<Event>> events = window->PollEvents();

			for (auto& event : events)
			{
				imguiContext->OnEvent(event);
				OnEvent(event);
			}

			if (mMinimized) continue;
			
			OnUpdate(dt);
			
			if (GraphicsContext::Get().NewFrame())
			{
				OnRender(dt);

				WeakRef<Scene> scene = mEngineContext->GetScene();
				std::vector<Ref<Semaphore>> waitSemaphores;

				Renderer::Get().Render();

				imguiContext->NewFrame();
				OnUIRender(dt);
				imguiContext->EndFrame({ waitSemaphores });

				GraphicsContext::Get().EndFrame({ imguiContext->GetRenderSemaphore() });
			}

			auto end = std::chrono::high_resolution_clock::now();
			auto diff = end - start;
			dt = diff.count() * 1e-9f;
			mRunning = window->WindowOpen();
		}

		while (!mLayerStack.empty()) PopLayer();

		mEngineContext = nullptr;
	}

	void Application::PopLayer()
	{
		mLayerStack.PopLayer();
	}

	void Application::OnEvent(Ref<Event> event)
	{
		switch (event->Type)
		{
		case EventType::WindowResizeEvent:
		{
			Ref<WindowResizeEvent> resizeEvent = event;
			GraphicsContext::Get().ResizeSwapchain(resizeEvent->Width, resizeEvent->Height);
			if (resizeEvent->Width == 0 || resizeEvent->Height == 0)
				mMinimized = true;
			else
				mMinimized = false;
		}
		break;
		}

		for (auto it = mLayerStack.begin(); it != mLayerStack.end(); it++)
		{
			if (event->IsHandled())
				break;

			(*it)->OnEvent(event);
		}
	}

	void Application::OnRender(float dt)
	{
		for (auto it = mLayerStack.begin(); it != mLayerStack.end(); it++)
		{
			(*it)->OnRender(dt);
		}
	}

	void Application::OnUIRender(float dt)
	{
		for (auto it = mLayerStack.begin(); it != mLayerStack.end(); it++)
		{
			(*it)->OnUIRender(dt);
		}
	}

	void Application::OnUpdate(float dt)
	{
		for (auto it = mLayerStack.begin(); it != mLayerStack.end(); it++)
		{
			(*it)->OnUpdate(dt);
		}
	}
}