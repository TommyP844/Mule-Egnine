
#include "Application/Application.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "Graphics/Context/GraphicsContext.h"

#include "imgui.h"

// Asset Loaders
#include "Asset/Loader/SceneLoader.h"
#include "Asset/Loader/ModelLoader.h"

// Events
#include "Application/Events/Event.h"
#include "Application/Events/WindowResizeEvent.h"

// STD
#include <vector>
#include <chrono>

namespace Mule
{
	Application::Application()
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
		float ms = 1.f;
		float maxFPS = 1.f;
		while (mRunning)
		{
			auto start = std::chrono::high_resolution_clock::now();

			window->PollEvents();
			const std::vector<Ref<Event>>& events = window->GetFrameEvents();

			for (auto& event : events)
			{
				switch (event->Type)
				{
				case EventType::WindowResize:
				{
					Ref<WindowResizeEvent> resizeEvent = event;
					graphicsContext->ResizeSwapchain(resizeEvent->Width, resizeEvent->Height);
					imguiContext->Resize(resizeEvent->Width, resizeEvent->Height);
				}
					break;
				default:
					break;
				}
			}

			graphicsContext->BeginFrame();

			Ref<Scene> scene = mEngineContext->GetScene();
			if (scene)
			{
				sceneRenderer->Render(scene);
			}

			// ImGui UI render
			{
				imguiContext->NewFrame();

				for (auto it = mLayerStack.begin(); it != mLayerStack.end(); it++)
				{
					(*it)->OnUIRender();
				}

				ImGui::Begin("Frame Counter");

				float fps = 1.f / (ms / 1000.f);
				if (fps > maxFPS) maxFPS = fps;
				ImGui::Text("Frame Time : %.3fms", ms);
				ImGui::Text("FPS        : %.2f", fps);
				ImGui::Text("Highest FPS: %.2f", maxFPS);

				ImGui::End();

				//std::vector<Ref<Semaphore>> waitSemaphores;
				//if (scene)
				//{
				//	waitSemaphores.push_back(sceneRenderer->GetCurrentFrameRenderFinishedSemaphore());
				//}

				imguiContext->EndFrame();
			}

			auto semaphore = imguiContext->GetRenderSemaphore();
			graphicsContext->EndFrame({ semaphore });

			mRunning = window->WindowOpen();
			auto end = std::chrono::high_resolution_clock::now();
			auto diff = end - start;
			ms = diff.count() * 1e-6f;
		}
	}

	void Application::PopLayer()
	{
		mLayerStack.PopLayer();
	}
}