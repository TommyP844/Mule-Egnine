#pragma once

#include <vulkan/vulkan.h>

namespace Mule
{

	class CommandBuffer
	{
	public:
		CommandBuffer(VkCommandBuffer commandbuffer);
		~CommandBuffer() {}

		VkCommandBuffer GetHandle() const { return mCommandBuffer; }

		void Begin();
		void End();


	private:
		VkCommandBuffer mCommandBuffer;
	};
}