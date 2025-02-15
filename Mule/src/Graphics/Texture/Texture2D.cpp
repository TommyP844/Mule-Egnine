#include "Graphics/Texture/Texture2D.h"

#include "Graphics/Context/GraphicsContext.h"

#include "Graphics/imguiImpl/imgui_impl_vulkan.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	Texture2D::Texture2D(WeakRef<GraphicsContext> context, void* data, int width, int height, TextureFormat format, TextureFlags flags)
		:
		ITexture(context)
	{
		Initialize(data, width, height, 1, 1, format, flags);

		mImGuiID = (ImTextureID)ImGui_ImplVulkan_AddTexture(mContext->GetLinearSampler(), mVulkanImage.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	Texture2D::Texture2D(WeakRef<GraphicsContext> context, void* data, int width, int height, int layers, TextureFormat format, TextureFlags flags)
		:
		ITexture(context)
	{
		Initialize(data, width, height, 1, layers, format, flags);

		mImGuiID = (ImTextureID)ImGui_ImplVulkan_AddTexture(mContext->GetLinearSampler(), mVulkanImage.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	Texture2D::Texture2D(WeakRef<GraphicsContext> context, const std::string& name, void* data, int width, int height, TextureFormat format, TextureFlags flags)
		:
		ITexture(context, name)
	{
		Initialize(data, width, height, 1, 1, format, flags);

		mImGuiID = (ImTextureID)ImGui_ImplVulkan_AddTexture(mContext->GetLinearSampler(), mVulkanImage.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	Texture2D::Texture2D(WeakRef<GraphicsContext> context, const fs::path& filepath, void* data, int width, int height, TextureFormat format, TextureFlags flags)
		:
		ITexture(context, filepath)
	{
		Initialize(data, width, height, 1, 1, format, flags);

		mImGuiID = (ImTextureID)ImGui_ImplVulkan_AddTexture(mContext->GetLinearSampler(), mVulkanImage.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	Texture2D::~Texture2D()
	{

	}
}