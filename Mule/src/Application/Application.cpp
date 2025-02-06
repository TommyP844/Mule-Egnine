
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
		Ref<Window> window = mEngineContext->GetWindow();
		Ref<ImGuiContext> imguiContext = mEngineContext->GetImGuiContext();
		Ref<GraphicsContext> graphicsContext = mEngineContext->GetGraphicsContext();
		Ref<SceneRenderer> sceneRenderer = mEngineContext->GetSceneRenderer();

		float dt = 1.f;

		while (mRunning)
		{
			auto start = std::chrono::high_resolution_clock::now();

			std::vector<Ref<Event>> events = window->PollEvents();

			for (auto& event : events)
			{
				OnEvent(event);
			}

			if (mMinimized) continue;

			if (graphicsContext->BeginFrame())
			{
				OnRender(dt);

				WeakRef<Scene> scene = mEngineContext->GetScene();
				std::vector<Ref<Semaphore>> waitSemaphores;
				if (scene)
				{
					waitSemaphores.push_back(sceneRenderer->GetCurrentFrameRenderFinishedSemaphore());
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
		case EventType::WindowResize:
		{
			Ref<WindowResizeEvent> resizeEvent = event;
			mEngineContext->GetGraphicsContext()->ResizeSwapchain(resizeEvent->Width, resizeEvent->Height);
			mEngineContext->GetImGuiContext()->Resize(resizeEvent->Width, resizeEvent->Height);
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
}