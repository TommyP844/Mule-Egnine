#pragma once

#include "Application/Window.h"

#include "Graphics/API/CommandAllocator.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/API/GraphicsQueue.h"
#include "Graphics/API/Fence.h"
#include "Graphics/API/Semaphore.h"

#include "Graphics/imguiImpl/imgui_impl_vulkan.h"

// Std
#include <array>
#include <vector>

namespace Mule
{
	class ImGuiContext
	{
	public:
		ImGuiContext(Ref<Window> window);
		~ImGuiContext();

		void NewFrame();
		void EndFrame(const std::vector<Ref<Semaphore>>& waitSemaphores = {});
		void Resize(uint32_t width, uint32_t height);

		void OnEvent(Ref<Event> event);

		Ref<Semaphore> GetRenderSemaphore() const { return mFrameData[mFrameIndex].Semaphore; }

	private:
		GraphicsAPI mAPI;
		void InitForVulkan();

		Ref<Window> mWindow;
		Ref<GraphicsQueue> mGraphicsQueue;

		struct FrameData
		{
			Ref<CommandAllocator> CommandPool;
			Ref<CommandBuffer> CommandBuffer;
			Ref<Semaphore> Semaphore;
			Ref<Fence> Fence;
		};
		std::array<FrameData, 2> mFrameData;
		uint32_t mFrameIndex;

		// Key Map
		ImGuiKey mKeyMap[1024];
	};
}