#include "Graphics/API/Vulkan/Execution/VulkanQueue.h"

#include "Graphics/API/Vulkan/VulkanContext.h"

#include "Graphics/API/Vulkan/Execution/VulkanCommandBuffer.h"
#include "Graphics/API/Vulkan/Syncronization/VulkanFence.h"
#include "Graphics/API/Vulkan/Syncronization/VulkanSemaphore.h"
#include "Graphics/API/Vulkan/Syncronization/VulkanTimelineSemaphore.h"

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

    void VulkanQueue::Submit(Ref<CommandBuffer> commandBuffer, Ref<TimelineSemaphore> semaphore, uint64_t waitValue, uint64_t signalValue, Ref<Fence> fence)
    {
        WeakRef<VulkanTimelineSemaphore> timelineSemaphore = semaphore;
        WeakRef<VulkanCommandBuffer> vulkanCommandBuffer = commandBuffer;

        VkCommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferInfo.pNext = nullptr;
        commandBufferInfo.commandBuffer = vulkanCommandBuffer->GetHandle();
        commandBufferInfo.deviceMask = 0;

        VkSemaphoreSubmitInfo semaphoreSignalInfo{};
        semaphoreSignalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSignalInfo.semaphore = timelineSemaphore->GetSemaphore();
        semaphoreSignalInfo.value = signalValue;
        semaphoreSignalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        semaphoreSignalInfo.deviceIndex = 0;

        VkSemaphoreSubmitInfo semaphoreWaitInfo{};
        semaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaphoreWaitInfo.semaphore = timelineSemaphore->GetSemaphore();
        semaphoreWaitInfo.value = waitValue;
        semaphoreWaitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        semaphoreWaitInfo.deviceIndex = 0;
        
        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = 1;
        submitInfo.pWaitSemaphoreInfos = &semaphoreWaitInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &semaphoreSignalInfo;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferInfo;
        
        VkFence vkFence = VK_NULL_HANDLE;
        if (fence)
        {
            Ref<VulkanFence> vulkanFence = fence;
            vkFence = vulkanFence->GetHandle();
        }
        
        vkQueueSubmit2KHR(mQueue, 1, &submitInfo, vkFence);
    }

    void VulkanQueue::Submit(Ref<CommandBuffer> commandBuffer, Ref<TimelineSemaphore> waitSemaphore, uint64_t waitValue, Ref<TimelineSemaphore> signalSemaphore, uint64_t signalValue, Ref<Fence> fence)
    {
        WeakRef<VulkanTimelineSemaphore> waitTimelineSemaphore = waitSemaphore;
        WeakRef<VulkanTimelineSemaphore> signalTimelineSemaphore = signalSemaphore;
        WeakRef<VulkanCommandBuffer> vulkanCommandBuffer = commandBuffer;

        VkCommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferInfo.pNext = nullptr;
        commandBufferInfo.commandBuffer = vulkanCommandBuffer->GetHandle();
        commandBufferInfo.deviceMask = 0;

        VkSemaphoreSubmitInfo semaphoreSignalInfo{};
        semaphoreSignalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSignalInfo.semaphore = signalTimelineSemaphore->GetSemaphore();
        semaphoreSignalInfo.value = signalValue;
        semaphoreSignalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        semaphoreSignalInfo.deviceIndex = 0;

        VkSemaphoreSubmitInfo semaphoreWaitInfo{};
        semaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaphoreWaitInfo.semaphore = waitTimelineSemaphore->GetSemaphore();
        semaphoreWaitInfo.value = waitValue;
        semaphoreWaitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        semaphoreWaitInfo.deviceIndex = 0;

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = 1;
        submitInfo.pWaitSemaphoreInfos = &semaphoreWaitInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &semaphoreSignalInfo;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferInfo;

        VkFence vkFence = VK_NULL_HANDLE;
        if (fence)
        {
            Ref<VulkanFence> vulkanFence = fence;
            vkFence = vulkanFence->GetHandle();
        }

        vkQueueSubmit2(mQueue, 1, &submitInfo, vkFence);
    }
}
