#pragma once

#include "Asset/Asset.h"

#include <RmlUi/Core.h>

namespace Mule::UI
{
	class Scene : public Asset<AssetType::UIScene>
	{
	public:
		Scene(const fs::path& filepath);
		~Scene();

		// TODO: events

		void Render();

	private:
		bool mValid;
		Rml::Context* mContext;
		Rml::ElementDocument* mDocument;
	};
}