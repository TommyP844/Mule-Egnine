
#include "Graphics/API/Vulkan/Texture/VulkanTextureCube.h"

#include "Graphics/API/Vulkan/VulkanTypeConversion.h"

#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"

namespace Mule::Vulkan
{
	VulkanTextureCube::VulkanTextureCube(const std::string& name, const Buffer& buffer, uint32_t width, TextureFormat format, TextureFlags flags)
		:
		TextureCube(name, format, flags)
	{
		InitCube(
			VK_IMAGE_TYPE_2D,
			GetVulkanFormat(format),
			width,
			width,
			1,
			1,
			6,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_VIEW_TYPE_CUBE
			);


		VulkanStagingBuffer stagingBuffer(buffer);

		VulkanContext& context = VulkanContext::Get();
		auto cmd = context.BeginSingleTimeCommandBuffer();
		context.TransitionImageLayout(cmd, this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 1, 0, 6);

		for (uint32_t i = 0; i < 6; i++)
		{
			uint32_t bufferOffset = i * width * width * GetFormatSize(format);


			VkBufferImageCopy region{};

			region.bufferImageHeight = 0;
			region.bufferRowLength = 0;
			region.bufferOffset = bufferOffset;
			region.imageExtent = { width, width, 1 };
			region.imageOffset = { 0, 0, 0 };
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.baseArrayLayer = i;
			region.imageSubresource.layerCount = 1;
			region.imageSubresource.mipLevel = 0;

			context.CopyBufferToImage(cmd, stagingBuffer.GetBuffer(), GetVulkanImage().Image, region);
		}

		context.TransitionImageLayout(cmd, this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 1, 0, 6);

		context.EndSingleTimeCommandBuffer(cmd);

		SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	uint32_t VulkanTextureCube::GetWidth()
	{
		return mWidth;
	}

	uint32_t VulkanTextureCube::GetHeight()
	{
		return mWidth;
	}

	uint32_t VulkanTextureCube::GetDepth()
	{
		return 1;
	}

	uint32_t VulkanTextureCube::GetMipLevels()
	{
		return mMipLevels;
	}

	uint32_t VulkanTextureCube::GetArrayLayers()
	{
		return mArrayLayers;
	}

	ImTextureID VulkanTextureCube::GetImGuiID(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return ImTextureID();
	}

	WeakRef<TextureView> VulkanTextureCube::GetView(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return WeakRef<TextureView>();
	}
}