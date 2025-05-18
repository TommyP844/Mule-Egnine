#pragma once

#include "Graphics/API/Texture2DArray.h"

#include "IVulkanTexture.h"

namespace Mule::Vulkan
{
	class VulkanTexture2DArray : public Texture2DArray, public IVulkanTexture
	{
	public:
		explicit VulkanTexture2DArray(const std::string& name, const Buffer&, int width, int height, uint32_t layers, TextureFormat format, TextureFlags flags);
		virtual ~VulkanTexture2DArray();

		uint32_t GetWidth() override;
		uint32_t GetHeight() override;
		uint32_t GetDepth() override;
		uint32_t GetMipLevels() override;
		uint32_t GetArrayLayers() override;
		void Resize(uint32_t width, uint32_t height) override;

		ImTextureID GetImGuiID(uint32_t mipLevel = 0, uint32_t arrayLayer = 0) const override;
		WeakRef<TextureView> GetView(uint32_t mipLevel, uint32_t arrayLayer = 0) const override;
		WeakRef<TextureView> GetMipView(uint32_t mipLevel) override;
		void TransitionImageLayoutImmediate(ImageLayout newLayout) override;
		Buffer ReadTextureData(uint32_t mipLevel = 0) override;
		void WriteMipLevel(uint32_t mipLevel, const Buffer& data) override;

	private:
		bool mHasMips;
	};
}
