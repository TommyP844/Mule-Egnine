#pragma once

#include <string>


namespace Mule
{
#define MULE_ENUM_OPERATORS(type) \
	static type operator&(const type& lhs, const type& rhs) \
	{ \
		return static_cast<type>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); \
	} \
	static type operator|(const type& lhs, const type& rhs) \
	{ \
		return static_cast<type>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
	} \

	enum class TextureFlags
	{
		None,
		TransferSrc,
		TransferDst,
		GenerateMips,
		RenderTarget,
		DepthAttachment,
		StorageImage
	};

	enum class TextureType
	{
		TextureType_2D,
		TextureType_2DArray,
		TextureType_3D,
		TextureType_Cube,
		TextureType_CubeArray,
	};

	enum class IndexType
	{
		Size_16Bit,
		Size_32Bit
	};

	enum class TextureFormat : uint32_t
	{
		R_8U,
		BGRA_8U,
		RGBA_8U,
		RGB_8U,

		R_32UI,
		RG_32UI,

		D_32F,
		D_24S8,

		RGBA_16F,

		R_32F,
		RGBA_32F,
		RGBA_32S,
		RGBA_32U,

		R_64UI,

		NONE
	};

	enum class ImageLayout : uint32_t
	{
		Undefined			= 0,
		TransferSrc			= 1 << 1,
		TransferDst			= 1 << 2,
		ColorAttachment		= 1 << 3,
		DepthAttachment		= 1 << 4,
		ShaderReadOnly		= 1 << 5,
		General				= 1 << 6
	};

	enum class DescriptorType
	{
		Texture,
		UniformBuffer,
		StorageImage
	};

	enum class ShaderStage
	{
		None					= 0,
		Vertex					= 1 << 1,
		Fragment				= 1 << 2,
		Geometry				= 1 << 3,
		TesselationControl		= 1 << 4,
		TesselationEvaluation	= 1 << 5,

		Compute					= 1 << 6,
	};

	enum class CullMode {
		Back,
		Front,
		None
	};

	enum class FillMode {
		Solid,
		Line,
		Point
	};

	enum class ShaderResourceType
	{
		UniformBuffer,
		Sampler,
		StorageImage
	};
	
	static std::string GetTextureFormatName(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R_8U: return "R8U";
		case TextureFormat::BGRA_8U: return "BGRA8U";
		case TextureFormat::RGBA_8U: return "RGBA8U";
		case TextureFormat::RGB_8U: return "RGB8U";
		case TextureFormat::D_32F: return "Depth32F";
		case TextureFormat::D_24S8: return "D24S8";
		case TextureFormat::RGBA_16F: return "RGBA16F";
		case TextureFormat::RGBA_32F: return "RGBA32F";
		case TextureFormat::RGBA_32S: return "RGBA32S";
		case TextureFormat::RGBA_32U: return "RGBA32U";
		case TextureFormat::NONE: return "NONE";
		}
		return "Unknown";
	}

	static uint32_t GetFormatSize(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R_8U:
			return 1;
		case TextureFormat::RGB_8U:
			return 3;
		case TextureFormat::BGRA_8U:
		case TextureFormat::RGBA_8U:
		case TextureFormat::D_32F:
		case TextureFormat::D_24S8:
			return 4;
		case TextureFormat::RGBA_16F:
			return 8;
		case TextureFormat::RGBA_32S:
		case TextureFormat::RGBA_32U:
		case TextureFormat::RGBA_32F:
			return 16;
		case TextureFormat::NONE:
		default:
			return 0;
		}
		return 0;
	}

	MULE_ENUM_OPERATORS(TextureFlags);
	MULE_ENUM_OPERATORS(ShaderStage);
}