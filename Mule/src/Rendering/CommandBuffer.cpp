#include "Rendering/CommandBuffer.h"

#include "Rendering/CommandPool.h"

namespace Mule
{
	CommandBuffer::CommandBuffer(VkCommandBuffer commandbuffer)
		:
		mCommandBuffer(commandbuffer)
	{
	}

	void CommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo beginInfo{};

		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;
		beginInfo.pNext = nullptr;

		vkBeginCommandBuffer(mCommandBuffer, nullptr);

		vkResetCommandBuffer(mCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	void CommandBuffer::End()
	{
		vkEndCommandBuffer(mCommandBuffer);
	}
}