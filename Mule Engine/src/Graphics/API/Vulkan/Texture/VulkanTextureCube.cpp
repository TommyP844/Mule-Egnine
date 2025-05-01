
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

		if ((flags & TextureFlags::GenerateMips) == TextureFlags::GenerateMips)
		{
			mMipLevels = std::log2(width) + 1;
		}
		else
		{
			mMipLevels = 1;
		}

		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		if ((flags & TextureFlags::StorageImage) == TextureFlags::StorageImage)
			usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

		InitCube(
			VK_IMAGE_TYPE_2D,
			GetVulkanFormat(format),
			width,
			width,
			1,
			mMipLevels,
			6,
			usageFlags,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_VIEW_TYPE_CUBE
			);

		
		VulkanContext& context = VulkanContext::Get();
		auto cmd = context.BeginSingleTimeCommandBuffer();
		
		std::vector<Ref<VulkanStagingBuffer>> stagingBuffers;

		if (buffer)
		{
			context.TransitionImageLayout(cmd, this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, mMipLevels, 0, 6);

			for (uint32_t i = 0; i < 6; i++)
			{
				uint32_t bufferOffset = i * width * width * GetFormatSize(format);
				auto stagingBuffer = MakeRef<VulkanStagingBuffer>(buffer);
				stagingBuffers.push_back(stagingBuffer);

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

				context.CopyBufferToImage(cmd, stagingBuffer->GetBuffer(), GetVulkanImage().Image, region);
			}

			context.TransitionImageLayout(cmd, this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, mMipLevels, 0, 6);

			SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else
		{
			if ((flags & TextureFlags::StorageImage) == TextureFlags::StorageImage)
			{
				context.TransitionImageLayout(cmd, this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 0, mMipLevels, 0, 6);
				SetImageLayout(VK_IMAGE_LAYOUT_GENERAL);
			}
			else
			{
				context.TransitionImageLayout(cmd, this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, mMipLevels, 0, 6);
				SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
		context.EndSingleTimeCommandBuffer(cmd);
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
		return GetTextureView(mipLevel, arrayLayer)->GetImGuiID();
	}

	WeakRef<TextureView> VulkanTextureCube::GetView(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return GetTextureView(mipLevel, arrayLayer);
	}

	WeakRef<TextureView> VulkanTextureCube::GetMipView(uint32_t mipLevel)
	{
		assert(mipLevel < mMipLevels && "Invalid mip level");

		if (!mMipViews[mipLevel])
		{
			VulkanContext& context = VulkanContext::Get();
			VkDevice device = context.GetDevice();

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.pNext = nullptr;
			viewInfo.flags = 0;
			viewInfo.image = GetVulkanImage().Image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			viewInfo.format = GetVulkanFormat(GetFormat());
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.subresourceRange.aspectMask = GetImageAspect();
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = mArrayLayers;
			viewInfo.subresourceRange.baseMipLevel = mipLevel;
			viewInfo.subresourceRange.levelCount = 1;

			VkImageView view;
			vkCreateImageView(device, &viewInfo, nullptr, &view);

			mMipViews[mipLevel] = MakeRef<VulkanTextureView>(view);
		}

		return mMipViews[mipLevel];
	}

	void VulkanTextureCube::TransitionImageLayoutImmediate(ImageLayout newLayout)
	{
		VulkanContext& context = VulkanContext::Get();

		auto cmd = context.BeginSingleTimeCommandBuffer();

		context.TransitionImageLayout(cmd, this, GetVulkanImage().Layout, Vulkan::GetImageLayout(newLayout), 0, mMipLevels, 0, mArrayLayers);
		SetImageLayout(Vulkan::GetImageLayout(newLayout));

		context.EndSingleTimeCommandBuffer(cmd);
	}

	Buffer VulkanTextureCube::ReadTextureData(uint32_t mipLevel)
	{
		uint32_t width = std::max(mWidth >> mipLevel, 1u);
		uint32_t height = std::max(mHeight >> mipLevel, 1u);

		uint32_t size = width * height * 6 * GetFormatSize(GetFormat());
		VulkanStagingBuffer stagingBuffer(size);

		VulkanContext& context = VulkanContext::Get();

		VkBufferImageCopy copyInfo{};
		copyInfo.bufferOffset = 0;
		copyInfo.bufferRowLength = 0;
		copyInfo.bufferImageHeight = 0;
		copyInfo.imageSubresource.aspectMask = GetImageAspect();
		copyInfo.imageSubresource.mipLevel = mipLevel;
		copyInfo.imageSubresource.baseArrayLayer = 0;
		copyInfo.imageSubresource.layerCount = 6;
		copyInfo.imageOffset = { 0, 0, 0 };
		copyInfo.imageExtent = { width, height, 1 };

		auto cmd = context.BeginSingleTimeCommandBuffer();
		auto oldLayout = GetVulkanImage().Layout;
		context.TransitionImageLayout(cmd, this, GetVulkanImage().Layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, mMipLevels, 0, mArrayLayers);
		context.CopyImageToBuffer(cmd, GetVulkanImage().Image, stagingBuffer.GetBuffer(), copyInfo);
		context.TransitionImageLayout(cmd, this, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, oldLayout, 0, mMipLevels, 0, mArrayLayers);
		context.EndSingleTimeCommandBuffer(cmd);

		Buffer buffer = stagingBuffer.ReadData(0, size);

		return buffer;
	}
	
	void VulkanTextureCube::WriteMipLevel(uint32_t mipLevel, const Buffer& data)
	{
		uint32_t width = std::max(mWidth >> mipLevel, 1u);
		uint32_t height = std::max(mHeight >> mipLevel, 1u);

		VulkanContext& context = VulkanContext::Get();
		auto cmd = context.BeginSingleTimeCommandBuffer();

		VkBufferImageCopy copyInfo{};
		copyInfo.bufferOffset = 0;
		copyInfo.bufferRowLength = 0;
		copyInfo.bufferImageHeight = 0;
		copyInfo.imageSubresource.aspectMask = GetImageAspect();
		copyInfo.imageSubresource.mipLevel = mipLevel;
		copyInfo.imageSubresource.baseArrayLayer = 0;
		copyInfo.imageSubresource.layerCount = 6;
		copyInfo.imageExtent = { width, height, 1 };
		copyInfo.imageOffset = { 0, 0, 0 };

		VulkanStagingBuffer stagingBuffer(data);
		auto oldLayout = GetImageLayout();
		context.TransitionImageLayout(cmd, this, oldLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevel, 1, 0, mArrayLayers);
		context.CopyBufferToImage(cmd, stagingBuffer.GetBuffer(), GetVulkanImage().Image, copyInfo);
		context.TransitionImageLayout(cmd, this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, oldLayout, mipLevel, 1, 0, mArrayLayers);

		context.EndSingleTimeCommandBuffer(cmd);
	}
}