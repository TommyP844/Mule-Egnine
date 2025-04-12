#pragma once

#include "WeakRef.h"
#include "GraphicsAPI.h"
#include "Application/Window.h"

namespace Mule
{
	class GraphicsContext
	{
	public:
		static void Init(GraphicsAPI api, WeakRef<Window> window);
		static void Shutdown();
		static GraphicsContext& Get();

		void NewFrame();
		void EndFrame();

		inline GraphicsAPI GetAPI() const { return mAPI; }

	private:
		GraphicsContext(GraphicsAPI api, WeakRef<Window> window);

		static GraphicsContext* mInstance;

		GraphicsAPI mAPI;

	};
}