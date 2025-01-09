#pragma once

// Engine
#include "Ref.h"

#include "Window.h"
#include "Layer/ILayer.h"
#include "Layer/LayerStack.h"
#include "ApplicationData.h"

// Submodules
#include "spdlog/spdlog.h"

// STD
#include <stack>

namespace Mule
{
	class Application
	{
	public:
		Application();
		~Application();

		void Run();

		template<typename T>
		void PushLayer()
		{
			mLayerStack.PushLayer<T>(mApplicationData);
		}

		void PopLayer();

	private:
		Ref<ApplicationData> mApplicationData;
		bool mRunning;
		Ref<Window> mWindow;

		LayerStack mLayerStack;
	};
}