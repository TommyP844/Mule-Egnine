#pragma once

#include <chrono>

namespace Mule
{
	class Timer
	{
	public:
		Timer() = default;
		~Timer() = default;

		void Start()
		{
			mTime = 0.f;
			mStart = std::chrono::high_resolution_clock::now();
		}

		void Stop()
		{
			auto diff = std::chrono::high_resolution_clock::now() - mStart;
			mTime = static_cast<double>(diff.count()) * 1e-9;
		}

		double Query() const { return mTime; }

	private:
		std::chrono::high_resolution_clock::time_point mStart;
		double mTime;
	};
}