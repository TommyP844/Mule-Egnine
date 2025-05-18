#pragma once

#include "Graphics/API/GraphicsCore.h"

#include <Volk/volk.h>

#include <assert.h>

namespace Mule::Vulkan
{
	constexpr VkFormat GetVulkanFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R_8U:		return VK_FORMAT_R8_UNORM;
		case TextureFormat::BGRA_8U:	return VK_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::RGBA_8U:	return VK_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGB_8U:	return VK_FORMAT_R8G8B8_UNORM;
		case TextureFormat::R_32UI:	return VK_FORMAT_R32_UINT;
		case TextureFormat::RG_32UI:	return VK_FORMAT_R32G32_UINT;
		case TextureFormat::D_32F:	return VK_FORMAT_D32_SFLOAT;
		case TextureFormat::D_24S8:	return VK_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::RGBA_16F:	return VK_FORMAT_R16G16B16A16_SFLOAT;
		case TextureFormat::R_32F:	return VK_FORMAT_R32_SFLOAT;
		case TextureFormat::RGBA_32F:	return VK_FORMAT_R32G32B32A32_SFLOAT;
		case TextureFormat::RGBA_32S:	return VK_FORMAT_R32G32B32A32_SINT;
		case TextureFormat::RGBA_32U:	return VK_FORMAT_R32G32B32A32_UINT;
		case TextureFormat::R_64UI:	return VK_FORMAT_R64_UINT;
		case TextureFormat::RGB_32F:		return VK_FORMAT_R32G32B32_SFLOAT;
		case Mule::TextureFormat::NONE: 
		default:
			assert(false && "Invalid texture format for vulkan");
			break;
		}
	}

	constexpr VkCompareOp GetCompareOp(DepthFunc func)
	{
		switch (func)
		{
		case Mule::DepthFunc::Less: return VK_COMPARE_OP_LESS;
		case Mule::DepthFunc::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
		case Mule::DepthFunc::Equal: return VK_COMPARE_OP_EQUAL;
		case Mule::DepthFunc::Greater: return VK_COMPARE_OP_GREATER;
		case Mule::DepthFunc::GraterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
		default:
			assert(false && "Invalid DepthFunc");
			break;
		}
	}

	constexpr VkCullModeFlags GetCullMode(CullMode mode)
	{
		switch (mode)
		{
		case Mule::CullMode::Back:	return VK_CULL_MODE_BACK_BIT;
		case Mule::CullMode::Front:	return VK_CULL_MODE_FRONT_BIT;
		case Mule::CullMode::None:	return VK_CULL_MODE_NONE;
		default:
			assert(false && "Invalid vulkan cull mode");
		}
	}

	constexpr VkPolygonMode GetPolygonMode(FillMode mode)
	{
		switch (mode)
		{
		case Mule::FillMode::Solid:		return VK_POLYGON_MODE_FILL;
		case Mule::FillMode::Line:		return VK_POLYGON_MODE_LINE;
		case Mule::FillMode::Point:		return VK_POLYGON_MODE_POINT;
		default:
			assert(false && "Invalid vulkan polygon mode");
		}
	}

	constexpr VkShaderStageFlags GetShaderStage(ShaderStage stage)
	{
		// TODO: handle multiple stages or'd together
		switch (stage)
		{
		case ShaderStage::Vertex:						return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::Fragment:						return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::Geometry:						return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ShaderStage::TesselationControl:			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case ShaderStage::TesselationEvaluation:		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ShaderStage::Compute:						return VK_SHADER_STAGE_COMPUTE_BIT;
		}

		// Assume we have multiple flags
		VkShaderStageFlags flags = (VkShaderStageFlagBits)0;

		if ((stage & ShaderStage::Vertex) == ShaderStage::Vertex) flags |= VK_SHADER_STAGE_VERTEX_BIT;
		if ((stage & ShaderStage::Fragment) == ShaderStage::Fragment) flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if ((stage & ShaderStage::Geometry) == ShaderStage::Geometry) flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if ((stage & ShaderStage::TesselationControl) == ShaderStage::TesselationControl) flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if ((stage & ShaderStage::TesselationEvaluation) == ShaderStage::TesselationEvaluation) flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		if ((stage & ShaderStage::Compute) == ShaderStage::Compute) flags |= VK_SHADER_STAGE_COMPUTE_BIT;

		return flags;
	}

	constexpr VkIndexType GetIndexFormat(IndexType type)
	{
		switch (type)
		{
		case Mule::IndexType::Size_16Bit: return VK_INDEX_TYPE_UINT16;
		case Mule::IndexType::Size_32Bit: return VK_INDEX_TYPE_UINT32;
		default:
			assert(false && "Invalid vulkan index type");
		}
	}

	constexpr VkDescriptorType GetResourceType(ShaderResourceType type)
	{
		switch (type)
		{
		case Mule::ShaderResourceType::UniformBuffer:	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case Mule::ShaderResourceType::Sampler:			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case Mule::ShaderResourceType::StorageImage:	return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		}
	}

	constexpr VkImageLayout GetImageLayout(ImageLayout layout)
	{
		switch (layout)
		{
		case Mule::ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
		case Mule::ImageLayout::TransferSrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case Mule::ImageLayout::TransferDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case Mule::ImageLayout::ColorAttachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case Mule::ImageLayout::DepthAttachment: return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		case Mule::ImageLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case Mule::ImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
		default:
			assert(false && "Invalid vulkan image layout");
			break;
		}
	}

	constexpr ImageLayout GetImageLayout(VkImageLayout layout)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED: return Mule::ImageLayout::Undefined;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return Mule::ImageLayout::TransferSrc;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return Mule::ImageLayout::TransferDst;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return Mule::ImageLayout::ColorAttachment;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return Mule::ImageLayout::DepthAttachment;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return Mule::ImageLayout::ShaderReadOnly;
		case VK_IMAGE_LAYOUT_GENERAL: return Mule::ImageLayout::General;
		default:
			assert(false && "Invalid vulkan image layout");
			break;
		}
	}

	constexpr VkDescriptorType GetDescriptorType(DescriptorType type)
	{
		switch (type)
		{
		case Mule::DescriptorType::Texture: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case Mule::DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case Mule::DescriptorType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		default:
			assert(false && "Invalid vulkan descriptor type");
			break;
		}
	}

	constexpr VkFilter GetFilter(SamplerFilterMode mode)
	{
		switch (mode)
		{
		case Mule::SamplerFilterMode::Linear: return VK_FILTER_LINEAR;
		case Mule::SamplerFilterMode::Nearest: return VK_FILTER_NEAREST;
		default:
			assert(false && "Invalid vulkan filter");
			break;
		}
	}

	constexpr VkSamplerMipmapMode GetMipMapMode(MipMapMode mode)
	{
		switch (mode)
		{
		case Mule::MipMapMode::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case Mule::MipMapMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		default:
			assert(false && "Invalid vulkan mipmap mode");
			break;
		}
	}

	constexpr VkSamplerAddressMode GetAddressMode(SamplerAddressMode mode)
	{
		switch (mode)
		{
		case Mule::SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case Mule::SamplerAddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		default:
			assert(false && "Invalid vulkan sampler address mode");
			break;
		}
	}

	constexpr VkBorderColor GetBorderColor(SamplerBorderColor color)
	{
		switch (color)
		{
		case SamplerBorderColor::None: return VK_BORDER_COLOR_MAX_ENUM;
		case SamplerBorderColor::White: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		case SamplerBorderColor::Black: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		default:
			assert(false && "Invalid vulkan border color");
			break;
		}
	}
}