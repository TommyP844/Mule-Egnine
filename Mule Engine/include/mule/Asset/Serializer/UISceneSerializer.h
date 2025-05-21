#pragma once

#include "Asset/Serializer/IAssetSerializer.h"

#include "Graphics/UI/UIScene.h"

#include <yaml-cpp/yaml.h>

namespace Mule
{
	class UISceneSerializer : public IAssetSerializer<UIScene, AssetType::UIScene>
	{
	public:
		UISceneSerializer(Ref<ServiceManager> serviceManager);
		virtual ~UISceneSerializer();

		Ref<UIScene> Load(const fs::path& filepath) override;
		void Save(Ref<UIScene> asset) override;

	private:
		YAML::Node SerializeElement(Ref<UIElement> element);
	};
}
