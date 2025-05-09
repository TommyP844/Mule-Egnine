#pragma once

#include <string>
#include <assert.h>

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
		None				= 0,
		TransferSrc			= 1 << 1,
		TransferDst			= 1 << 2,
		GenerateMips		= 1 << 3,
		RenderTarget		= 1 << 4,
		DepthAttachment		= 1 << 5,
		StorageImage		= 1 << 6,
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
		RGB_32F,

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

	static std::string ToString(TextureFormat format)
	{
		switch (format)
		{
		case Mule::TextureFormat::R_8U: return "R_8U";
		case Mule::TextureFormat::BGRA_8U: return "BGRA_8U";
		case Mule::TextureFormat::RGBA_8U: return "RGBA_8U";
		case Mule::TextureFormat::RGB_8U: return "RGB_8U";
		case Mule::TextureFormat::R_32UI: return "R_32UI";
		case Mule::TextureFormat::RG_32UI: return "RG_32UI";
		case Mule::TextureFormat::D_32F: return "D_32F";
		case Mule::TextureFormat::D_24S8: return "D_24S8";
		case Mule::TextureFormat::RGBA_16F: return "RGBA_16F";
		case Mule::TextureFormat::R_32F: return "R_32F";
		case Mule::TextureFormat::RGBA_32F: return "RGBA_32F";
		case Mule::TextureFormat::RGBA_32S: return "RGBA_32S";
		case Mule::TextureFormat::RGBA_32U: return "RGBA_32U";
		case Mule::TextureFormat::R_64UI: return "R_64UI";
		case Mule::TextureFormat::NONE:
			assert(false && "Unknown TextureFormat");
			break;
		}
	}

	static TextureFormat ToFormat(const std::string& formatName)
	{
		if (formatName == "R_8U")         return Mule::TextureFormat::R_8U;
		if (formatName == "BGRA_8U")      return Mule::TextureFormat::BGRA_8U;
		if (formatName == "RGBA_8U")      return Mule::TextureFormat::RGBA_8U;
		if (formatName == "RGB_8U")       return Mule::TextureFormat::RGB_8U;
		if (formatName == "R_32UI")       return Mule::TextureFormat::R_32UI;
		if (formatName == "RG_32UI")      return Mule::TextureFormat::RG_32UI;
		if (formatName == "D_32F")        return Mule::TextureFormat::D_32F;
		if (formatName == "D_24S8")       return Mule::TextureFormat::D_24S8;
		if (formatName == "RGBA_16F")     return Mule::TextureFormat::RGBA_16F;
		if (formatName == "R_32F")        return Mule::TextureFormat::R_32F;
		if (formatName == "RGBA_32F")     return Mule::TextureFormat::RGBA_32F;
		if (formatName == "RGBA_32S")     return Mule::TextureFormat::RGBA_32S;
		if (formatName == "RGBA_32U")     return Mule::TextureFormat::RGBA_32U;
		if (formatName == "R_64UI")       return Mule::TextureFormat::R_64UI;

		assert(false && "Unknown format name");
		return Mule::TextureFormat::NONE;
	}
}