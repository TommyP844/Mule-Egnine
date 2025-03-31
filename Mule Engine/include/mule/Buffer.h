#pragma once

#include <typeinfo>

namespace Mule
{
	class Buffer
	{
	public:
		Buffer()
			:
			mData(nullptr),
			mSize(0)
		{ }

		Buffer(void* data, uint64_t size)
			:
			mData(data),
			mSize(size)
		{}

		Buffer(uint64_t size)
			:
			mSize(size),
			mData(nullptr)
		{
			Allocate(size);
		}

		Buffer(const Buffer& other)
			:
			mData(other.mData),
			mSize(other.mSize)
		{ }

		void Allocate(size_t size)
		{
			Release();
			mSize = size;
			mData = new uint8_t[size];
		}

		void Release()
		{
			delete[] mData;
			mData = nullptr;
			mSize = 0;
		}

		Buffer& operator=(const Buffer& other)
		{
			mData = other.mData;
			mSize = other.mSize;

			return *this;
		}

		uint64_t GetSize() const { return mSize; }
		void* GetData() const { return mData; }

		template<typename T>
		T* As() const { return (T*)mData; }

	private:
		void* mData;
		uint64_t mSize;
	};
}