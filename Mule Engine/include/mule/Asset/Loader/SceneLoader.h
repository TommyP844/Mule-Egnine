
#include "AssetLoader.h"

#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"

#include "yaml-cpp/node/node.h"


namespace Mule
{
	class ScriptContext;
	class EngineContext;

	class SceneLoader : public IAssetLoader<Scene, AssetType::Scene>
	{
	public:
		SceneLoader(WeakRef<EngineContext> engineContext, WeakRef<ScriptContext> scriptContext);
		virtual ~SceneLoader() {}


		virtual Ref<Scene> LoadText(const fs::path& filepath) override;
		virtual void SaveText(Ref<Scene> asset) override;

		virtual Ref<Scene> LoadBinary(const Buffer& filepath) override;
		virtual void SaveBinary(Ref<Scene> asset) override;

	private:
		WeakRef<ScriptContext> mScriptContext;
		WeakRef<EngineContext> mEngineContext;

		YAML::Node SerializeEntityYAML(Entity e);
		Entity DeSerializeEntityYAML(const YAML::Node& node, WeakRef<Scene> scene);

		void DeserializeScriptComponentYAML(const YAML::Node& node, Entity e);
	};
}