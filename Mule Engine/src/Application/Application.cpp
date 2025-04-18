
#include "Application/Application.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "Graphics/Context/GraphicsContext.h"

#include "imgui.h"

// Events
#include "Application/Events/Event.h"
#include "Application/Events/WindowResizeEvent.h"

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
		engineDescription.GraphicsDescription.AppName = "Mule Editor";
		engineDescription.GraphicsDescription.AppVersion = 1;
		engineDescription.GraphicsDescription.EngineName = "Mule Engine";
		engineDescription.GraphicsDescription.EngineVersion = 1;
		engineDescription.ProjectPath = "C:/Development/Mule Projects/Test Project";
		// engineDescription.GraphicsDescription.Window = mWindow; -- Will be set by engine context
#ifdef _DEBUG
		engineDescription.GraphicsDescription.EnableDebug = true;
#else
		graphicsContextDesc.EnableDebug = false;
#endif

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
		WeakRef<GraphicsContext> graphicsContext = mEngineContext->GetGraphicsContext();
		WeakRef<SceneRenderer> sceneRenderer = mEngineContext->GetSceneRenderer();

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
			
			if (graphicsContext->BeginFrame())
			{
				OnRender(dt);

				WeakRef<Scene> scene = mEngineContext->GetScene();
				std::vector<WeakRef<Semaphore>> waitSemaphores;
				if (scene)
				{
					waitSemaphores.push_back(sceneRenderer->GetSemaphore());
				}

				imguiContext->NewFrame();
				OnUIRender(dt);
				imguiContext->EndFrame({ waitSemaphores });

				graphicsContext->EndFrame({ imguiContext->GetRenderSemaphore() });
			}

			auto end = std::chrono::high_resolution_clock::now();
			auto diff = end - start;
			dt = diff.count() * 1e-9f;
			mRunning = window->WindowOpen();
		}
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
			mEngineContext->GetGraphicsContext()->ResizeSwapchain(resizeEvent->Width, resizeEvent->Height);
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