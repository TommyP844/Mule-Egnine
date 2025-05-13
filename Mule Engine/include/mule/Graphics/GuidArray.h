#pragma once

#include "Asset/Asset.h"

#include <unordered_map>
#include <vector>
#include <queue>

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
			mIndices.clear();
			mArray.clear();
		}

		uint32_t Insert(AssetHandle handle, const T& value)
		{
			uint32_t index = QueryIndex(handle);
			if (index != UINT_MAX)
				return index;
			if (!mFreeIndices.empty())
			{
				uint32_t index = mFreeIndices.front();
				mFreeIndices.pop();
				mIndices[handle] = index;
				mArray[index] = value;
				return index;
			}
			else
			{
				mArray.push_back(value);
				uint32_t index = mArray.size() - 1;
				mIndices[handle] = index;
				return index;
			}
			// Should never happen
			return -1;
		}

		uint32_t QueryIndex(AssetHandle handle)
		{
			if (!handle)
				return UINT32_MAX;
			auto iter = mIndices.find(handle);
			if (iter == mIndices.end())
				return UINT32_MAX;
			return iter->second;
		}

		void Update(uint32_t index, const T& data)
		{
			if (index >= mArray.size())
			{
				SPDLOG_WARN("Invalid index for GuidArray: {}", index);
				return;
			}
			mArray[index] = data;
		}

		void Remove(AssetHandle handle)
		{
			auto iter = mIndices.find(handle);
			if (iter == mIndices.end())
				return;

			mFreeIndices.push(iter->second);
			mIndices.erase(iter);
		}

		const T& Query(AssetHandle handle) const
		{
			uint32_t index = QueryIndex(handle);
			return mArray[index];
		}

		const T& Query(uint32_t index) const
		{
			return mArray[index];
		}

		const std::vector<T>& GetArray() const { return mArray; }

	private:
		std::unordered_map<AssetHandle, uint32_t> mIndices;
		std::vector<T> mArray;
		std::queue<uint32_t> mFreeIndices;
		
	};
}