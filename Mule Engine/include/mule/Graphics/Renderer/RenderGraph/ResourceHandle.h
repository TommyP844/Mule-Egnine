#pragma once

#include <string>
#include <random>

namespace Mule
{
	struct ResourceHandle
	{
		std::string Name;
		uint64_t Handle;

		ResourceHandle(const std::string& name, uint64_t handle)
			:
			Name(name)
		{
			Handle = std::hash<std::string>{}(name);
		}

		ResourceHandle()
			:
			Name("Null"),
			Handle(0)
		{
		}

		static ResourceHandle Null()
		{
			return ResourceHandle{ "Null", 0 };
		}

		static ResourceHandle Create(const std::string& name)
		{
			std::random_device rd;
			std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);
			uint64_t handle = dist(rd);

			return ResourceHandle{ name, handle };
		}

		bool operator==(const ResourceHandle& rhs) const
		{
			return Handle == rhs.Handle;
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