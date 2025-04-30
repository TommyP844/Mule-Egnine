#include "Graphics/API/Vulkan/Execution/VulkanQueue.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include "Graphics/API/Vulkan/Execution/VulkanCommandBuffer.h"
#include "Graphics/API/Vulkan/Syncronization/VulkanFence.h"
#include "Graphics/API/Vulkan/Syncronization/VulkanSemaphore.h"

namespace Mule::Vulkan
{
    VulkanQueue::VulkanQueue()
        :
        mQueue(VK_NULL_HANDLE)
    {
        mQueue = VulkanContext::Get().CreateQueue();
    }

    VulkanQueue::~VulkanQueue()
    {
        VulkanContext::Get().ReleaseQueue(mQueue);
    }

    void VulkanQueue::Submit(Ref<CommandBuffer> commandBuffer, const std::vector<Ref<Semaphore>>& waitSemaphores, const std::vector<Ref<Semaphore>>& signalSemaphores, Ref<Fence> fence)
    {
        std::vector<VkSemaphore> waitVkSemaphores(waitSemaphores.size());
        std::vector<VkSemaphore> signalVkSemaphores(signalSemaphores.size());

        for (uint32_t i = 0; i < waitSemaphores.size(); i++)
        {
            WeakRef<VulkanSemaphore> vulkanSemaphore = waitSemaphores[i];
            waitVkSemaphores[i] = vulkanSemaphore->GetHandle();
        }

        for (uint32_t i = 0; i < signalSemaphores.size(); i++)
        {
            WeakRef<VulkanSemaphore> vulkanSemaphore = signalSemaphores[i];
            signalVkSemaphores[i] = vulkanSemaphore->GetHandle();
        }

        Ref<VulkanCommandBuffer> vulkanCommandBuffer = commandBuffer;
        VkCommandBuffer cmd = vulkanCommandBuffer->GetHandle();

        VkFence vkFence = VK_NULL_HANDLE;
        if (fence)
        {
            Ref<VulkanFence> vulkanFence = fence;
            vkFence = vulkanFence->GetHandle();
        }

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo info{
            .sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext                  = nullptr,
            .waitSemaphoreCount     = static_cast<uint32_t>(waitVkSemaphores.size()),
            .pWaitSemaphores        = waitVkSemaphores.data(),
            .pWaitDstStageMask      = &waitStage,
            .commandBufferCount     = 1u,
            .pCommandBuffers        = &cmd,
            .signalSemaphoreCount   = static_cast<uint32_t>(signalVkSemaphores.size()),
            .pSignalSemaphores      = signalVkSemaphores.data(),
        };

        vkQueueSubmit(mQueue, 1, &info, vkFence);
    }
}
