
#include "Application/Application.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "Rendering/GraphicsContext.h"

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
		mWindow = MakeRef<Window>("Title");

		GraphicsContextDescription graphicsContextDesc{};
		graphicsContextDesc.AppName = "Mule Editor";
		graphicsContextDesc.AppVersion = 1;
		graphicsContextDesc.EngineName = "Mule Engine";
		graphicsContextDesc.EngineVersion = 1;
		graphicsContextDesc.Window = mWindow;
#ifdef _DEBUG
		graphicsContextDesc.EnableDebug = true;
#else
		graphicsContextDesc.EnableDebug = false;
#endif

		mGraphicsContext = MakeRef<GraphicsContext>(graphicsContextDesc);
		mImguiContext = MakeRef<ImGuiContext>(mGraphicsContext);

		mApplicationData = MakeRef<ApplicationData>();
		mApplicationData->SetGraphicsContext(mGraphicsContext);

		// Loaders
		mApplicationData->GetAssetManager()->RegisterLoader<SceneLoader>();
		mApplicationData->GetAssetManager()->RegisterLoader<ModelLoader>();
	}

	Application::~Application()
	{
		while (!mLayerStack.empty())
			mLayerStack.PopLayer();

		mApplicationData->Shutdown();

		mImguiContext = nullptr;
		mGraphicsContext = nullptr;
	}

	void Application::Run()
	{
		float ms = 1.f;
		float maxFPS = 1.f;
		while (mRunning)
		{
			auto start = std::chrono::high_resolution_clock::now();
			mWindow->PollEvents();
			const std::vector<Ref<Event>>& events = mWindow->GetFrameEvents();

			for (auto& event : events)
			{
				switch (event->Type)
				{
				case EventType::WindowResize:
				{
					Ref<WindowResizeEvent> resizeEvent = event;
					mGraphicsContext->ResizeSwapchain(resizeEvent->Width, resizeEvent->Height);
					mImguiContext->Resize(resizeEvent->Width, resizeEvent->Height);
				}
					break;
				default:
					break;
				}
			}

			mGraphicsContext->BeginFrame();

			// ImGui UI render
			{
 				mImguiContext->NewFrame();

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

				mImguiContext->EndFrame();
			}

			auto semaphore = mImguiContext->GetRenderSemaphore();
			mGraphicsContext->EndFrame({ semaphore });

			mRunning = mWindow->WindowOpen();
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