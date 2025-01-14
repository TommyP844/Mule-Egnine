
#include "Application/Application.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "Rendering/GraphicsContext.h"

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
	}

	Application::~Application()
	{
		while (!mLayerStack.empty())
			mLayerStack.PopLayer();

		mApplicationData->Shutdown();
	}

	void Application::Run()
	{
		while (mRunning)
		{
			glfwPollEvents();
			
			mGraphicsContext->BeginFrame();

			mImguiContext->NewFrame();
			for (auto it = mLayerStack.begin(); it != mLayerStack.end(); it++)
			{
				(*it)->OnUIRender();
			}
			mImguiContext->EndFrame();

			mGraphicsContext->EndFrame();

			mRunning = mWindow->WindowOpen();
		}
	}

	void Application::PopLayer()
	{
		mLayerStack.PopLayer();
	}
}