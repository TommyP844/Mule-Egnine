#pragma once

#include "ResourceType.h"

#include <string>
#include <random>

namespace Mule
{
	struct ResourceHandle
	{
		std::string Name;
		uint64_t Handle;
		ResourceType Type;

		ResourceHandle(const std::string& name, ResourceType type)
			:
			Name(name),
			Type(type)
		{
			Handle = std::hash<std::string>{}(name);
		}

		ResourceHandle()
			:
			Name("Null"),
			Handle(0)
		{
		}

		bool operator==(const ResourceHandle& rhs) const
		{
			return Handle == rhs.Handle;
		}

		bool operator!=(const ResourceHandle& rhs) const
		{
			return Handle != rhs.Handle;
		}

		operator bool() const {
			return Handle != 0;
		}
	};
}

namespace std
{
	template<>
	struct hash<Mule::ResourceHandle>
	{
		size_t operator()(const Mule::ResourceHandle& handle) const noexcept
		{
			return std::hash<uint64_t>{}(handle.Handle);
		}
	};
}