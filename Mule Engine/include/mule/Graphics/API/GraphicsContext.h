#pragma once

#include "Ref.h"
#include "GraphicsAPI.h"
#include "Application/Window.h"
#include "Graphics/API/Semaphore.h"

namespace Mule
{
	class GraphicsContext
	{
	public:
		static void Init(GraphicsAPI api, Ref<Window> window);
		static void Shutdown();
		static GraphicsContext& Get();

		bool NewFrame();
		void EndFrame(const std::vector<Ref<Semaphore>>& waitSemaphores);
		void ResizeSwapchain(uint32_t width, uint32_t height);
		void AwaitIdle();

		inline GraphicsAPI GetAPI() const { return mAPI; }

	private:
		GraphicsContext(GraphicsAPI api, Ref<Window> window);

		static GraphicsContext* mInstance;

		GraphicsAPI mAPI;

	};
}