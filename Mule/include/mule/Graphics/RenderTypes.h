#pragma once

#include <string>
#include <typeinfo>

#include <vulkan/vulkan.h>

namespace Mule
{
	enum class ImageLayout : uint32_t
	{
		TransferSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		TransferDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		ColorAttachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		DepthAttachment = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		ShaderReadOnly = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	enum class DescriptorType
	{
		Texture = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
	};

	enum class ShaderStage
	{
		Vertex = VK_SHADER_STAGE_VERTEX_BIT,
		Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
		Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
		TesselationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		TesselationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,

		Compute = VK_SHADER_STAGE_COMPUTE_BIT,

		None = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM
	};

	enum TextureFlags : uint32_t
	{
		GenerateMips = 1,
		RenderTarget = 2,
		DepthTexture = 4,
		None = 8
	};

	enum class TextureFormat : uint32_t
	{
		BGRA8U = VK_FORMAT_B8G8R8A8_UNORM,
		RGBA8U = VK_FORMAT_R8G8B8A8_UNORM,
		RGB8U = VK_FORMAT_R8G8B8_UNORM,
		D32F = VK_FORMAT_D32_SFLOAT,
		D24S8 = VK_FORMAT_D24_UNORM_S8_UINT,
		RGBA32F = VK_FORMAT_R32G32B32A32_SFLOAT,
		RGBA32S = VK_FORMAT_R32G32B32A32_SINT,
		RGBA32U = VK_FORMAT_R32G32B32A32_UINT,
		NONE = VK_FORMAT_UNDEFINED
	};

	static std::string GetTextureFormatName(TextureFormat format)
	{
		switch (format)
		{
		case Mule::TextureFormat::BGRA8U: return "BGRA8U";
		case Mule::TextureFormat::RGBA8U: return "RGBA8U";
		case Mule::TextureFormat::RGB8U: return "RGB8U";
		case Mule::TextureFormat::D32F: return "Depth32F";
		case Mule::TextureFormat::D24S8: return "D24S8";
		case Mule::TextureFormat::RGBA32F: return "RGBA32F";
		case Mule::TextureFormat::RGBA32S: return "RGBA32S";
		case Mule::TextureFormat::RGBA32U: return "RGBA32U";
		case Mule::TextureFormat::NONE: return "NONE";
		}
		return "Unknown";
	}

	static uint32_t GetFormatSize(TextureFormat format)
	{
		switch (format)
		{
		case Mule::TextureFormat::BGRA8U:
		case Mule::TextureFormat::RGBA8U:
		case Mule::TextureFormat::D32F:
		case Mule::TextureFormat::D24S8:
			return 4;
		case Mule::TextureFormat::RGB8U:
			return 3;
		case Mule::TextureFormat::NONE:
		default:
			return 0;
		}
		return 0;
	}

	struct Attachment
	{
		TextureFormat Format = TextureFormat::NONE;
	};

	struct VulkanImage
	{
		VkImage Image = VK_NULL_HANDLE;
		VkDeviceMemory Memory = VK_NULL_HANDLE;
		VkImageView ImageView = VK_NULL_HANDLE;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
}