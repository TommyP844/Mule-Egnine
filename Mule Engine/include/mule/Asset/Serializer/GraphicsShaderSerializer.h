#pragma once

#include "IAssetSerializer.h"
#include "WeakRef.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Shader/GraphicsShader.h"

namespace Mule
{
	class EngineContext;

	class GraphicsShaderSerializer : public IAssetSerializer<GraphicsShader, AssetType::Shader>
	{
	public:
		GraphicsShaderSerializer(WeakRef<EngineContext> context);
		virtual ~GraphicsShaderSerializer(){}

		virtual Ref<GraphicsShader> Load(const fs::path& filepath) override;
		virtual void Save(Ref<GraphicsShader> asset) override;

	private:
		WeakRef<EngineContext> mContext;
	};
}