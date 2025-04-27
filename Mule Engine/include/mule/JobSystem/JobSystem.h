#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <functional>

namespace Mule
{
	class JobSystem
	{
	public:
		JobSystem();
		~JobSystem();

		template<typename F, typename... Args>
		void PushJob(F&& func, Args&&... args)
		{
			mJobs.push([func, args...]() {
				func(std::forward<Args>(args)...);
				});
		}

	private:
		bool mRunning;
		std::mutex mMutex;

		void Worker();

		std::vector<std::thread> mThreads;
		std::queue<std::function<void()>> mJobs;
	};
}