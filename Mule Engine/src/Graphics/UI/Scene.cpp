#include "Graphics/UI/Scene.h"

#include <RmlUi/Core.h>

#include <spdlog/spdlog.h>

namespace Mule::UI
{

	Scene::Scene(const fs::path& filepath)
		:
		Asset(filepath),
		mValid(false)
	{
		std::string name = filepath.filename().replace_extension().string();
		mContext = Rml::CreateContext(name, Rml::Vector2i(1920, 1080));
		mDocument = mContext->LoadDocument(filepath.string());
		if (!mDocument)
		{
			SPDLOG_ERROR("Failed to load UI Scene: {}", filepath.string());
			return;
		}

		mValid = true;
	}

	Scene::~Scene()
	{
		mContext->UnloadAllDocuments();
	}

	void Scene::Render()
	{
		mContext->Update();
		mContext->Render();
	}
}