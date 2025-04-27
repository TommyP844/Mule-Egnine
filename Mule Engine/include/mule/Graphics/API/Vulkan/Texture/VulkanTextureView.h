#pragma once

#include "Graphics/API/TextureView.h"

#include <Volk/volk.h>
#include <imgui.h>

namespace Mule::Vulkan
{
	class VulkanTextureView : public TextureView
	{
	public:
		VulkanTextureView(VkImageView view);
		~VulkanTextureView();

		VkImageView GetView() const { return mView; }
		ImTextureID GetImGuiID() const override { return mImGuiID; }

	private:
		VkImageView mView;
		ImTextureID mImGuiID;
	};
}
