#pragma once

#include "IAssetSerializer.h"

#include "Graphics/UI/UITheme.h"

#include <yaml-cpp/yaml.h>
#include "Convert/YamlConvert.h"
#include "Convert/YamlUIConvert.h"

namespace Mule
{
	class ThemeSerializer : public IAssetSerializer<UITheme, AssetType::UITheme>
	{
	public:
		ThemeSerializer(Ref<ServiceManager> serviceManager);
		virtual ~ThemeSerializer();

		Ref<UITheme> Load(const fs::path& filepath) override;
		void Save(Ref<UITheme> asset) override;

	private:

		template<typename T>
		void SerializeStyle(Ref<T> style, YAML::Node& node)
		{
			for (auto state : AllUIStates)
			{
				YAML::Node stateNode = node[ToString(state)];
				auto vars = style->GetStateVars(state);
				stateNode = vars;
			}
		}
	};
}
