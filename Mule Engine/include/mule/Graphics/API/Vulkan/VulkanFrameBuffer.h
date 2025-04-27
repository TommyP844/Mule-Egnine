#pragma once

#include "Graphics/API/Framebuffer.h"
#include "Graphics/API/Vulkan/Texture/VulkanTexture2D.h"

namespace Mule::Vulkan
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferDescription& desc);
		virtual ~VulkanFramebuffer();


		void Resize(uint32_t width, uint32_t height) override;
		Ref<Texture2D> GetColorAttachment(uint32_t index) override;
		Ref<Texture2D> GetDepthAttachment() override;

	private:
		FramebufferDescription mDescription;

		std::vector<Ref<VulkanTexture2D>> mColorAttachments;
		Ref<VulkanTexture2D> mDepthAttachment;
	};
}