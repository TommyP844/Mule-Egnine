#pragma once

#include "GraphicsContext.h"
#include "CommandPool.h"
#include "CommandBuffer.h"

namespace Mule
{
	class ImGuiContext
	{
	public:
		ImGuiContext(Ref<GraphicsContext> graphicsContext);
		~ImGuiContext(){}

		void NewFrame();

		void EndFrame();

	private:
		WeakRef<GraphicsContext> mContext;
		WeakRef<GraphicsQueue> mGraphicsQueue;
		Ref<CommandPool> mCommandPool;
		Ref<CommandBuffer> mCommandBuffer;
	};
}