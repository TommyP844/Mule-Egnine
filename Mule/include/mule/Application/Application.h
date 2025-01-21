#pragma once

// Engine
#include "Ref.h"

#include "Window.h"
#include "Layer/ILayer.h"
#include "Layer/LayerStack.h"
#include "EngineContext.h"

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
			mLayerStack.PushLayer<T>(mEngineContext);
		}

		void PopLayer();

	private:
		bool mRunning;
		LayerStack mLayerStack;
		Ref<EngineContext> mEngineContext;
	};
}