#pragma once

#include <random>
#include <string>

namespace Mule
{
	struct AssetHandle
	{
		AssetHandle()
			:
			Handle(CreateHandle())
		{}

		AssetHandle(uint64_t handle)
			:
			Handle(handle)
		{}

		uint64_t Handle;

		static const AssetHandle Null()
		{
			return AssetHandle(0);
		}

		operator bool() const {
			return Handle != 0ull;
		}

		operator uint64_t() const {
			return Handle;
		}

		bool operator ==(const AssetHandle& rhs) const {
			return Handle == rhs.Handle;
		}

		bool operator !=(const AssetHandle& rhs) const {
			return Handle != rhs.Handle;
		}

		std::string ToString()
		{
			return std::to_string(Handle);
		}

	private:
		static uint64_t CreateHandle() {
			std::random_device rd;
			std::mt19937_64 rng(rd());

			uint64_t min = UINT16_MAX;
			uint64_t max = UINT64_MAX;

			std::uniform_int_distribution<uint64_t> dist(min, max);

			return dist(rng);
		}
	};
}

namespace std {
	template <>
	struct hash<Mule::AssetHandle> {
		std::size_t operator()(const Mule::AssetHandle& key) const {
			return std::hash<size_t>{}(key.Handle);
		}
	};
}