#pragma once

#include <type_traits> // uint64_t
#include <random>
#include <xhash>

namespace Mule
{
	struct UIHandle
	{
		UIHandle()
			:
			Handle(0)
		{
		}

		UIHandle(uint64_t handle)
			:
			Handle(handle)
		{
		}

		static UIHandle Create()
		{
			std::random_device rd;
			std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);

			return UIHandle(dist(rd));
		}

		uint64_t Handle;

		operator bool() const
		{
			return Handle != 0;
		}

		bool operator==(const UIHandle& rhs) const
		{
			return Handle == rhs.Handle;
		}
	};
}

namespace std {
	template <>
	struct hash<Mule::UIHandle> {
		std::size_t operator()(const Mule::UIHandle& key) const {
			return std::hash<size_t>{}(key.Handle);
		}
	};
}
