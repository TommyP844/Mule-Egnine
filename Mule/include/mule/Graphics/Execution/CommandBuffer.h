#pragma once

// Render Object
#include "Ref.h"
#include "WeakRef.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderPass.h"
#include "Graphics/SwapchainframeBuffer.h"
#include "Graphics/GraphicsShader.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/ComputeShader.h"

#include <vulkan/vulkan.h>

namespace Mule
{
	class CommandBuffer
	{
	public:
		CommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
		~CommandBuffer();
		void Begin();
		void End();

		void TransitionSwapchainFrameBufferForRendering(Ref<SwapchainFrameBuffer> fb);
		void TransitionSwapchainFrameBufferForPresent(Ref<SwapchainFrameBuffer> fb);

		// Framebuffers
		void BeginRenderPass(Ref<SwapchainFrameBuffer> framebuffer);
		void BeginRenderPass(Ref<FrameBuffer> framebuffer, Ref<RenderPass> renderPass);
		void NextPass();
		void EndRenderPass();

		// Texture
		void TranistionImageLayout(WeakRef<ITexture> texture, ImageLayout newLayout);

		// Shader
		void BindGraphicsPipeline(WeakRef<GraphicsShader> shader);
		void BindComputePipeline(WeakRef<ComputeShader> shader);
		void SetPushConstants(WeakRef<GraphicsShader> shader, ShaderStage stage, void* data, uint32_t size);
		void BindGraphicsDescriptorSet(Ref<GraphicsShader> shader, Ref<DescriptorSet> descriptorSet);
		void BindComputeDescriptorSet(WeakRef<ComputeShader> shader, Ref<DescriptorSet> descriptorSet);
		void Execute(uint32_t workGroupsX, uint32_t workGroupsY, uint32_t workGroupsZ);

		// Mesh
		void BindMesh(WeakRef<Mesh> mesh);
		void DrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount = 1);
		void BindAndDrawMesh(WeakRef<Mesh> mesh, uint32_t instanceCount);
		

		VkCommandBuffer GetHandle() const { return mCommandBuffer; }

	private:
		VkDevice mDevice;
		VkCommandPool mCommandPool;
		VkCommandBuffer mCommandBuffer;
	};
}
