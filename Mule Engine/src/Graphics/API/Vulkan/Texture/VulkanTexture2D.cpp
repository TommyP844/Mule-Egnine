#include "Graphics/API/Vulkan/Texture/VulkanTexture2D.h"

#include "Graphics/imguiImpl/imgui_impl_vulkan.h"

#include "Graphics/API/Vulkan/VulkanContext.h"
#include "Graphics/API/Vulkan/VulkanTypeConversion.h"
#include "Graphics/API/Vulkan/Buffer/VulkanStagingBuffer.h"

#include <spdlog/spdlog.h>

namespace Mule::Vulkan
{
	VulkanTexture2D::VulkanTexture2D(const std::string& name, const Buffer& buffer, int width, int height, TextureFormat format, TextureFlags flags)
		:
		Texture2D(name, format, flags)
	{
		mWidth = width;
		mHeight = height;
		mDepth = 1;
		mArrayLayers = 1;

		VulkanContext& context = VulkanContext::Get();

		if ((TextureFlags::GenerateMips & flags) == TextureFlags::GenerateMips)
		{
			mMipLevels = glm::log2(glm::max<float>(mWidth, mHeight));
			mHasMips = true;
		}
		else
		{
			mMipLevels = 1;
			mHasMips = false;
		}

		mIsDepthTexture = false;
				
		VkFormat textureFormat = GetVulkanFormat(format);
		VkImageAspectFlags aspectFlags = ((flags & TextureFlags::DepthAttachment) == TextureFlags::DepthAttachment) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget
			&& (flags & TextureFlags::DepthAttachment) == TextureFlags::DepthAttachment)
		{
			mIsDepthTexture = true;
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget) 
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if ((flags & TextureFlags::StorageImage) == TextureFlags::StorageImage)	usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

		bool success = Init(
			VK_IMAGE_TYPE_2D,
			textureFormat,
			mWidth,
			mHeight,
			1,
			mMipLevels,
			1,
			usageFlags,
			aspectFlags,
			VK_IMAGE_VIEW_TYPE_2D
		);

		auto cmd = context.BeginSingleTimeCommandBuffer();
		
		// We need this to stay alive untul the constructor leaves
		Ref<VulkanStagingBuffer> stagingBuffer;

		if (buffer)
		{
			// Transition every mip level to be transfer source
			context.TransitionImageLayout(
				cmd,
				this,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				0,
				mMipLevels,
				0,
				1);

			stagingBuffer = MakeRef<VulkanStagingBuffer>(buffer);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = aspectFlags;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { mWidth, mHeight, 1 };

			VkBuffer buf = stagingBuffer->GetBuffer();
			VkImage image = GetVulkanImage().Image;

			context.CopyBufferToImage(cmd, buf, image, region);

			if (mHasMips)
			{
				int32_t mipWidth = mWidth;
				int32_t mipHeight = mHeight;
				for (int i = 1; i < mMipLevels; i++)
				{
					// transition previous mip to transfer src
					context.TransitionImageLayout(
						cmd,
						this,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						i - 1,
						1,
						0,
						1);


					VkImageBlit blit{};
					blit.srcOffsets[0] = { 0, 0, 0 };
					blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
					blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.srcSubresource.mipLevel = i - 1;
					blit.srcSubresource.baseArrayLayer = 0;
					blit.srcSubresource.layerCount = 1;

					blit.dstOffsets[0] = { 0, 0, 0 };
					blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
					blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.dstSubresource.mipLevel = i;
					blit.dstSubresource.baseArrayLayer = 0;
					blit.dstSubresource.layerCount = 1;

					context.BlitMip(cmd, this, blit);


					if (mipWidth > 1) mipWidth /= 2;
					if (mipHeight > 1) mipHeight /= 2;
				}

				// transition last mip level to transfer src
				context.TransitionImageLayout(
					cmd,
					this,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					mMipLevels - 1,
					1,
					0,
					1);

				// Since we have mips we assume its going to be used as a sampler image, transition the enitre image to shader read only
				context.TransitionImageLayout(
					cmd,
					this,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					0,
					mMipLevels,
					0,
					1);

				SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			else
			{
				// we dont have mips but we do have data, transfer to shader read only
				context.TransitionImageLayout(
					cmd,
					this,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					mMipLevels - 1,
					1,
					0,
					1);

				SetImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
		else
		{
			bool isRenderTarget = (flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget;
			bool isDepth = (flags & TextureFlags::DepthAttachment) == TextureFlags::DepthAttachment;
			if (isRenderTarget)
			{
				if (isDepth)
				{
					context.TransitionImageLayout(
						cmd,
						this,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
						0, mMipLevels,
						0, 1);

					SetImageLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
				}
				else
				{
					context.TransitionImageLayout(
						cmd,
						this,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						0, mMipLevels,
						0, 1);

					SetImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				}
			}
			else
			{
				// Its not a render target so we assume some kind of storage image
				context.TransitionImageLayout(
					cmd,
					this,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_GENERAL,
					0, mMipLevels,
					0, 1);

				SetImageLayout(VK_IMAGE_LAYOUT_GENERAL);
			}
		}

		context.EndSingleTimeCommandBuffer(cmd);
	}

	VulkanTexture2D::~VulkanTexture2D()
	{

	}

	uint32_t VulkanTexture2D::GetWidth()
	{
		return mWidth;
	}

	uint32_t VulkanTexture2D::GetHeight()
	{
		return mHeight;
	}

	uint32_t VulkanTexture2D::GetDepth()
	{
		return mDepth;
	}

	uint32_t VulkanTexture2D::GetMipLevels()
	{
		return mMipLevels;
	}

	uint32_t VulkanTexture2D::GetArrayLayers()
	{
		return mArrayLayers;
	}

	ImTextureID VulkanTexture2D::GetImGuiID(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return GetTextureView(mipLevel, arrayLayer)->GetImGuiID();
	}
	
	WeakRef<TextureView> VulkanTexture2D::GetView(uint32_t mipLevel, uint32_t arrayLayer) const
	{
		return GetTextureView(mipLevel, arrayLayer);
	}

	WeakRef<TextureView> VulkanTexture2D::GetMipView(uint32_t mipLevel)
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
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
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
	
	void VulkanTexture2D::TransitionImageLayoutImmediate(ImageLayout newLayout)
	{
		VulkanContext& context = VulkanContext::Get();

		auto cmd = context.BeginSingleTimeCommandBuffer();

		context.TransitionImageLayout(cmd, this, GetVulkanImage().Layout, Vulkan::GetImageLayout(newLayout), 0, mMipLevels, 0, mArrayLayers);
		SetImageLayout(Vulkan::GetImageLayout(newLayout));

		context.EndSingleTimeCommandBuffer(cmd);
	}

	Buffer VulkanTexture2D::ReadTextureData(uint32_t mipLevel)
	{
		uint32_t width = std::max(mWidth >> mipLevel, 1u);
		uint32_t height = std::max(mHeight >> mipLevel, 1u);

		uint32_t size = width * height * GetFormatSize(GetFormat());
		VulkanStagingBuffer stagingBuffer(size);

		VulkanContext& context = VulkanContext::Get();

		VkBufferImageCopy copyInfo{};
		copyInfo.bufferOffset = 0;
		copyInfo.bufferRowLength = 0;
		copyInfo.bufferImageHeight = 0;
		copyInfo.imageSubresource.aspectMask = GetImageAspect();
		copyInfo.imageSubresource.mipLevel = mipLevel;
		copyInfo.imageSubresource.baseArrayLayer = 0;
		copyInfo.imageSubresource.layerCount = mArrayLayers;
		copyInfo.imageOffset = { 0, 0, 0 };
		copyInfo.imageExtent = { mWidth, mHeight, 1 };
		
		auto cmd = context.BeginSingleTimeCommandBuffer();
		context.CopyBufferToImage(cmd, stagingBuffer.GetBuffer(), GetVulkanImage().Image, copyInfo);
		context.EndSingleTimeCommandBuffer(cmd);

		Buffer buffer = stagingBuffer.ReadData(0, size);

		return buffer;
	}

	void VulkanTexture2D::WriteMipLevel(uint32_t mipLevel, const Buffer& data)
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
		copyInfo.imageSubresource.layerCount = mArrayLayers;
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