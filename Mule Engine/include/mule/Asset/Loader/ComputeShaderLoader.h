#pragma once

#include "AssetLoader.h"
#include "Graphics/Shader/ComputeShader.h"

namespace Mule
{
	class EngineContext;

	class ComputeShaderLoader : public IAssetLoader<ComputeShader, AssetType::ComputeShader>
	{
	public:
		ComputeShaderLoader(WeakRef<EngineContext> context);
		virtual ~ComputeShaderLoader();

		Ref<ComputeShader> LoadText(const fs::path& filepath) override;
		void SaveText(Ref<ComputeShader> asset) override;
		Ref<ComputeShader> LoadBinary(const Buffer& buffer) override;
		void SaveBinary(Ref<ComputeShader> asset) override;

	private:
		WeakRef<EngineContext> mContext;
	};
}
