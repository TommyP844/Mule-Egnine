#pragma once

// Render Object
#include "Ref.h"
#include "WeakRef.h"
#include "Graphics/Mesh.h"
#include "Graphics/RenderPass.h"
#include "Graphics/SwapchainframeBuffer.h"
#include "Graphics/GraphicsShader.h"
#include "Graphics/FrameBuffer.h"

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
		void BindPipeline(WeakRef<GraphicsShader> shader);

		template<typename T>
		void SetPushConstants(WeakRef<GraphicsShader> shader, ShaderStage stage, const T& value, uint32_t offset = 0);

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

#include "CommandBuffer.inl"