
#include "AssetLoader.h"

#include "ECS/Scene.h"

namespace Mule
{
	class SceneLoader : public IAssetLoader<Scene, AssetType::Scene>
	{
	public:
		SceneLoader() {}
		virtual ~SceneLoader() {}

		virtual Ref<Scene> Load(const fs::path& filepath) override;
	};
}