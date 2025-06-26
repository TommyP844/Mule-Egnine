#include "Asset/Serializer/UISceneSerializer.h"

namespace Mule
{
	UISceneSerializer::UISceneSerializer(Ref<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	UISceneSerializer::~UISceneSerializer()
	{
	}

	Ref<UI::Scene> UISceneSerializer::Load(const fs::path& filepath)
	{
		return MakeRef<UI::Scene>(filepath);
	}

	void UISceneSerializer::Save(Ref<UI::Scene> asset)
	{
	}
}