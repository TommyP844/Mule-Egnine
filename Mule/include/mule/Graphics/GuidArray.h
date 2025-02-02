#pragma once

#include "Asset/Asset.h"

#include <unordered_map>
#include <vector>

namespace Mule
{
	template<typename T>
	class GuidArray
	{
	public:
		GuidArray()
		{
			Clear();
		}

		~GuidArray(){}

		void Clear()
		{
			mSizePtr = 0;
			mIndices.clear();
			mArray.clear();
		}

		uint32_t Insert(AssetHandle handle, const T& value)
		{
			uint32_t index = QueryIndex(handle);
			if (index != UINT_MAX)
				return index;
			mArray.push_back(value);
			mIndices[handle] = mSizePtr++;
			return mSizePtr - 1;
		}

		uint32_t QueryIndex(AssetHandle handle)
		{
			auto iter = mIndices.find(handle);
			if (iter == mIndices.end())
				return UINT_MAX;
			return iter->second;
		}

		const T& Query(AssetHandle handle)
		{
			uint32_t index = QueryIndex(handle);
			return mArray[index];
		}

		const T& Query(uint32_t index)
		{
			return mArray[index];
		}

		const std::vector<T>& GetArray() const { return mArray; }

	private:
		uint32_t mSizePtr;
		std::unordered_map<AssetHandle, uint32_t> mIndices;
		std::vector<T> mArray;
	};
}