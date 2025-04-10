
#include "IAssetSerializer.h"

#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"

#include "yaml-cpp/node/node.h"


namespace Mule
{
	class ScriptContext;
	class EngineContext;

	class SceneSerializer : public IAssetSerializer<Scene, AssetType::Scene>
	{
	public:
		SceneSerializer(WeakRef<EngineContext> engineContext);
		virtual ~SceneSerializer() {}


		virtual Ref<Scene> Load(const fs::path& filepath) override;
		virtual void Save(Ref<Scene> asset) override;


	private:
		WeakRef<EngineContext> mEngineContext;

		YAML::Node SerializeEntityYAML(Entity e);
		Entity DeSerializeEntityYAML(const YAML::Node& node, WeakRef<Scene> scene);
	};
}