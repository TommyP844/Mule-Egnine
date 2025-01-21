#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace Mule
{
	enum class AttributeType : uint32_t
	{
		Float = VK_FORMAT_R32_SFLOAT,
		Vec2 = VK_FORMAT_R32G32_SFLOAT,
		Vec3 = VK_FORMAT_R32G32B32_SFLOAT,
		Vec4 = VK_FORMAT_R32G32B32A32_SFLOAT,

		Int = VK_FORMAT_R32_SINT,
		Int2 = VK_FORMAT_R32G32_SINT,
		Int3 = VK_FORMAT_R32G32B32_SINT,
		Int4 = VK_FORMAT_R32G32B32A32_SINT,

		UInt = VK_FORMAT_R32_UINT,
		UInt2 = VK_FORMAT_R32G32_UINT,
		UInt3 = VK_FORMAT_R32G32B32_UINT,
		UInt4 = VK_FORMAT_R32G32B32A32_UINT,
	};

	static uint32_t GetAttributeSize(AttributeType type)
	{
		switch (type)
		{
		case Mule::AttributeType::UInt:
		case Mule::AttributeType::Float:
		case Mule::AttributeType::Int:
			return 1;
		case Mule::AttributeType::Vec2:
		case Mule::AttributeType::UInt2:
		case Mule::AttributeType::Int2:
			return 2;
		case Mule::AttributeType::Int3:
		case Mule::AttributeType::Vec3:
		case Mule::AttributeType::UInt3:
			return 3;
		case Mule::AttributeType::Int4:
		case Mule::AttributeType::Vec4:
		case Mule::AttributeType::UInt4:
			return 4;
		default:
			break;
		}
	}

	class VertexLayout
	{
	public:
		VertexLayout() {}
		~VertexLayout() {}

		VertexLayout& AddAttribute(AttributeType type)
		{
			mTypes.push_back(type);
			return *this;
		}

		const std::vector<AttributeType>& GetAttributes() const { return mTypes; }

	private:
		std::vector<AttributeType> mTypes;
	};
}