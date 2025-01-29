#pragma once

#include "Event.h"

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

namespace Mule
{
	class DropFileEvent : public Event
	{
	public:
		DropFileEvent(uint32_t pathCount, const char** paths) : Event(EventType::DropFile) 
		{
			for (uint32_t i = 0; i < pathCount; i++)
			{
				mPaths.push_back(std::string(paths[i]));
			}
		}


		const std::vector<fs::path>& GetPaths() const { return mPaths; }
	private:
		std::vector<fs::path> mPaths;

	};
}