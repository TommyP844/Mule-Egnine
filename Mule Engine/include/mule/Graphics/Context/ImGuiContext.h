#pragma once

#include "GraphicsContext.h"
#include "Graphics/Execution/CommandPool.h"
#include "Graphics/Execution/CommandBuffer.h"

#include "Graphics/imguiImpl/imgui_impl_vulkan.h"

// Std
#include <array>
#include <vector>

namespace Mule
{
	class ImGuiContext
	{
	public:
		ImGuiContext(WeakRef<GraphicsContext> graphicsContext);
		~ImGuiContext();

		void NewFrame();
		void EndFrame(const std::vector<WeakRef<Semaphore>>& waitSemaphores = {});
		void Resize(uint32_t width, uint32_t height);

		void OnEvent(Ref<Event> event);

		Ref<Semaphore> GetRenderSemaphore() const { return mFrameData[mFrameIndex].Semaphore; }

	private:
		WeakRef<GraphicsContext> mContext;
		WeakRef<GraphicsQueue> mGraphicsQueue;
		Ref<SwapchainFrameBuffer> mFrameBuffer;

		struct FrameData
		{
			Ref<CommandPool> CommandPool;
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