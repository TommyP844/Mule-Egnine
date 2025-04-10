#pragma once

#include "IAssetSerializer.h"
#include "Graphics/Shader/ComputeShader.h"

namespace Mule
{
	class EngineContext;

	class ComputeShaderSerializer : public IAssetSerializer<ComputeShader, AssetType::ComputeShader>
	{
	public:
		ComputeShaderSerializer(WeakRef<EngineContext> context);
		virtual ~ComputeShaderSerializer();

		Ref<ComputeShader> Load(const fs::path& filepath) override;
		void Save(Ref<ComputeShader> asset) override;

	private:
		WeakRef<EngineContext> mContext;
	};
}
