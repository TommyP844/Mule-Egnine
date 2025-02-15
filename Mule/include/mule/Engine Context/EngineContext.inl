#pragma once

#include "Graphics/Texture/TextureCube.h"
#include "Graphics/EnvironmentMap.h"

namespace Mule
{
#pragma region Asset Manager
	template<typename T>
	inline WeakRef<T> Mule::EngineContext::LoadAsset(const fs::path& filepath)
	{
		Ref<IAsset> asset = mAssetManager->LoadAsset<T>(filepath);
		switch (asset->GetType())
		{
		case AssetType::Texture:
		{
			WeakRef<ITexture> texture = asset;
			mSceneRenderer->AddTexture(texture);
		}
			break;
		case AssetType::Material:
		{
			WeakRef<Material> material = asset;
			mSceneRenderer->AddMaterial(asset);
		}
			break;
		case AssetType::EnvironmentMap:
		{
			WeakRef<EnvironmentMap> envMap = asset;
			WeakRef<TextureCube> cubeMap = GetAsset<TextureCube>(envMap->GetCubeMapHandle());
			WeakRef<TextureCube> diffuseIBLMap = GetAsset<TextureCube>(envMap->GetDiffuseIBLMap());
			WeakRef<TextureCube> prefilterMap = GetAsset<TextureCube>(envMap->GetPreFilterMap());
			mSceneRenderer->AddTexture(cubeMap);
			mSceneRenderer->AddTexture(diffuseIBLMap);
			mSceneRenderer->AddTexture(prefilterMap);
		}
			break;
		}
		return asset;
	}

	template<typename T>
	inline WeakRef<T> EngineContext::GetAsset(AssetHandle handle)
	{
		return mAssetManager->GetAsset<T>(handle);
	}

	template<typename T>
	inline void EngineContext::InsertAsset(Ref<T> asset)
	{
		switch (asset->GetType())
		{
		case AssetType::Texture:
		{

			WeakRef<ITexture> texture = asset;
			mSceneRenderer->AddTexture(texture);
		}
			break;
		case AssetType::Material:
		{
			WeakRef<Material> material = asset;
			mSceneRenderer->AddMaterial(asset);
		}
			break;
		case AssetType::EnvironmentMap:
		{
			WeakRef<EnvironmentMap> envMap = asset;
			WeakRef<TextureCube> cubeMap = GetAsset<TextureCube>(envMap->GetCubeMapHandle());
			WeakRef<TextureCube> diffuseIBLMap = GetAsset<TextureCube>(envMap->GetDiffuseIBLMap());
			WeakRef<TextureCube> prefilterMap = GetAsset<TextureCube>(envMap->GetPreFilterMap());
			mSceneRenderer->AddTexture(cubeMap);
			mSceneRenderer->AddTexture(diffuseIBLMap);
			mSceneRenderer->AddTexture(prefilterMap);
		}
			break;
		}
		mAssetManager->InsertAsset<T>(asset);
	}

	template<typename T>
	inline void EngineContext::SaveAssetText(AssetHandle handle)
	{
		mAssetManager->SaveAssetText<T>(handle);
	}

#pragma endregion
}