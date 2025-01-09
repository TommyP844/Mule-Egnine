#pragma once

#include <random>

namespace Mule
{
	class Guid
	{
	public:
		Guid()
			:
			mHandle(GenerateHandle())
		{}

		Guid(const Guid& guid)
			:
			mHandle(guid.mHandle)
		{}

		Guid(size_t handle)
			:
			mHandle(handle)
		{}

		bool operator==(const Guid& other)
		{
			return mHandle == other.mHandle;
		}

		bool operator!=(const Guid& other)
		{
			return mHandle != other.mHandle;
		}

		operator size_t()
		{
			return mHandle;
		}

		operator size_t() const
		{
			return mHandle;
		}

	private:
		size_t mHandle;

		size_t GenerateHandle()
		{
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<size_t> dis;

			return dis(gen);
		}
	};
}