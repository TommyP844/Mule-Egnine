#pragma once

#include "Graphics/API/GraphicsCore.h"
#include "Graphics/API/Vulkan/Texture/VulkanImage.h"
#include <glm/glm.hpp>
#include <imgui.h>

/*
* 
* Everty type of vulkan texture should inherit from this class
* This way all texture logic is defined in one place
* Make it abstract so its not tighly coupled to one use case

*/

namespace Mule::Vulkan
{
	enum class TextureType {
		Type_1D,
		Type_1DArray,
		Type_2D,
		Type_2DArray,
		Type_3D,
		Type_Cube,
		Type_CubeArray,
	};


	class IVulkanTexture
	{
	protected:
		IVulkanTexture() = default;
		virtual ~IVulkanTexture();

		void SetImageLayout(VkImageLayout layout) { mVulkanImage.Layout = layout; }

		VkImage GetImage() const { return mVulkanImage.Image; }
		VkImageView GetImageView() const { return mVulkanImage.ImageView; }
		VkImageView GetMipLayerImageView(uint32_t mipLevel, uint32_t layer = 0) const;
		VkImageView GetMipImageView(uint32_t mipLevel) const;
		ImTextureID GetImGuiMipLayerID(uint32_t mipLevel, uint32_t layer = 0) const;
		ImTextureID GetImGuiMipID(uint32_t mipLevel) const;

		void GenerateMips();

		const VulkanImage& GetVulkanImage() const { return mVulkanImage; }

		TextureType GetTextureType() const { return mTextureType; }

		VkSampler GetSampler() const { return mSampler; }

	protected:
		void Initialize(void* data, uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, TextureFormat format, TextureFlags flags);

		VkDevice mDevice;
		VulkanImage mVulkanImage;
		VkSampler mSampler;

		struct MipView
		{
			ImTextureID ImGuiMipId;
			VkImageView View;

			// Layers
			std::vector<VkImageView> LayerViews;
			std::vector<ImTextureID> ImGuiLayerIDs;
		};

		std::vector<MipView> mMipViews;
		
		bool mIsDepthTexture;
		uint32_t mWidth, mHeight, mDepth, mMips, mLayers;
		TextureFormat mFormat;
		TextureType mTextureType;
		TextureFlags mFlags;
	};
}