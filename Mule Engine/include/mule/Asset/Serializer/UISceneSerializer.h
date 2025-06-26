#pragma once

#include "Graphics/UI/Scene.h"
#include "IAssetSerializer.h"

namespace Mule
{
	class UISceneSerializer : public IAssetSerializer<UI::Scene, AssetType::UIScene>
	{
	public:
		UISceneSerializer(Ref<ServiceManager> serviceManager);
		~UISceneSerializer();

		Ref<UI::Scene> Load(const fs::path& filepath) override;
		void Save(Ref<UI::Scene> asset) override;
	};
}
