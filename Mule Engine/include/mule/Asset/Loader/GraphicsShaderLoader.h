#pragma once

#include "AssetLoader.h"
#include "WeakRef.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/Shader/GraphicsShader.h"

namespace Mule
{
	class EngineContext;

	class GraphicsShaderLoader : public IAssetLoader<GraphicsShader, AssetType::Shader>
	{
	public:
		GraphicsShaderLoader(WeakRef<EngineContext> context);
		virtual ~GraphicsShaderLoader(){}

		virtual Ref<GraphicsShader> LoadText(const fs::path& filepath) override;
		virtual void SaveText(Ref<GraphicsShader> asset) override;
		virtual Ref<GraphicsShader> LoadBinary(const Buffer& buffer) override;
		virtual void SaveBinary(Ref<GraphicsShader> asset) override;

	private:
		WeakRef<EngineContext> mContext;
	};
}