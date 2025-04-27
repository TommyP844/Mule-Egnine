#pragma once

#include <functional>
#include <random>

namespace Mule::RenderGraph
{
	template<typename T>
	struct ResourceHandle
	{
		ResourceHandle() : Handle(0) { }
		ResourceHandle(uint64_t handle) : Handle(handle) { }

		uint64_t Handle;

		static ResourceHandle Null()
		{
			return ResourceHandle(0);
		}

		static ResourceHandle Create()
		{
			ResourceHandle handle;

			std::random_device rd;
			std::uniform_int_distribution<uint64_t> rng(1, UINT64_MAX);

			handle.Handle = rng(rd);

			return handle;
		}

		bool operator==(const ResourceHandle& rhs) const
		{
			return Handle == rhs.Handle;
		}

		operator bool()
		{
			return Handle != 0;
		}
	};
}

namespace std {
	template<typename T>
	struct hash<Mule::RenderGraph::ResourceHandle<T>>
	{
		std::size_t operator()(const Mule::RenderGraph::ResourceHandle<T>& handle) const noexcept
		{
			return std::hash<uint64_t>{}(handle.Handle);
		}
	};
}
