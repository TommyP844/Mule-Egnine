#include "JobSystem/JobSystem.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	JobSystem::JobSystem()
		:
		mRunning(true)
	{
		uint32_t threadCount = std::thread::hardware_concurrency();
		threadCount = threadCount < 4 ? threadCount : 4;

		for (uint32_t i = 0; i < threadCount; i++)
		{
			SPDLOG_INFO("Adding Thread to Job System");
			mThreads.emplace_back(std::thread(&JobSystem::Worker, this));
		}
	}

	JobSystem::~JobSystem()
	{
		mRunning = false;
		for (auto& thread : mThreads)
		{
			SPDLOG_INFO("Joining job system thread");
			thread.join();
		}
	}

	void JobSystem::Worker()
	{
		while (mRunning)
		{
			std::function<void()> func;
			{
				std::unique_lock<std::mutex> lock(mMutex);
				if (mJobs.empty())
					continue;

				func = mJobs.front();
				mJobs.pop();
			}
			func();
		}
	}
}