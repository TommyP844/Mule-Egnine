
#include "Application/Application.h"
#include "Rendering/MasterRenderer.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"

#include "imgui.h"

// Asset Loaders
#include "Asset/Loader/SceneLoader.h"
#include "Asset/Loader/ModelLoader.h"

namespace Mule
{
	Application::Application()
	{
#ifdef _DEBUG
		spdlog::set_pattern("[%H:%M:%S %z] [%n] [thread %t] %v");
		SPDLOG_INFO("Application Started");
#endif
		mWindow = Ref<Window>::Make("Title");
		MasterRenderer::Init(mWindow);

		mApplicationData = Ref<ApplicationData>::Make();

		WeakRef<AssetManager> assetManager = mApplicationData->GetAssetManager();
		assetManager->RegisterLoader<SceneLoader>();
		assetManager->RegisterLoader<ModelLoader>();
	}

	Application::~Application()
	{
		while (!mLayerStack.empty())
			mLayerStack.PopLayer();

		mApplicationData->Shutdown();
		MasterRenderer::Shutdown();
	}

	void Application::Run()
	{
		while (mRunning)
		{
			glfwPollEvents();
			
			MasterRenderer& renderer = MasterRenderer::Get();
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