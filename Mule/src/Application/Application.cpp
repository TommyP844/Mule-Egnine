
#include "Application/Application.h"
#include "Rendering/Renderer.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"

#include "imgui.h"

// Asset Loaders
#include "Asset/Loader/SceneLoader.h"

namespace Mule
{
	Application::Application()
	{
#ifdef _DEBUG
#ifdef _WIN32
		SetConsoleOutputCP(CP_UTF8);
#endif
		spdlog::set_pattern("[%H:%M:%S %z] [%n] [thread %t] %v");
		SPDLOG_INFO("Application Started");
#endif
		mWindow = Ref<Window>::Make("Title");
		Renderer::Init(mWindow);

		mApplicationData = Ref<ApplicationData>::Make();

		WeakRef<AssetManager> assetManager = mApplicationData->GetAssetManager();
		assetManager->RegisterLoader<SceneLoader>();
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		while (mRunning)
		{
			glfwPollEvents();
			
			Renderer& renderer = Renderer::Get();
			renderer.NewFrame();
			renderer.NewImGuiFrame();

			for (auto it = mLayerStack.begin(); it != mLayerStack.end(); it++)
			{
				(*it)->OnUIRender();
			}

			renderer.RenderImGuiFrame();
			renderer.RenderFrame();

			mRunning = mWindow->WindowOpen();
		}
	}

	void Application::PopLayer()
	{
		mLayerStack.PopLayer();
	}
}