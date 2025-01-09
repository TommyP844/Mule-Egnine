#include "Asset/Loader/SceneLoader.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	Ref<Scene> SceneLoader::Load(const fs::path& filepath)
	{
		SPDLOG_INFO("Loading Scene: {}", filepath.string());
		std::this_thread::sleep_for(std::chrono::seconds(5));
		SPDLOG_INFO("Finished loading Scene: {}", filepath.string());
		return Ref<Scene>::Make();
	}
}