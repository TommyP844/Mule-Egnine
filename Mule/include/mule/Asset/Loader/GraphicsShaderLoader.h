#pragma once

#include "AssetLoader.h"
#include "WeakRef.h"
#include "Graphics/Context/GraphicsContext.h"
#include "Graphics/GraphicsShader.h"

namespace Mule
{
	class GraphicsShaderLoader : public IAssetLoader<GraphicsShader, AssetType::Shader>
	{
	public:
		GraphicsShaderLoader(WeakRef<GraphicsContext> context);
		virtual ~GraphicsShaderLoader(){}

		virtual Ref<GraphicsShader> LoadText(const fs::path& filepath) override;
		virtual void SaveText(Ref<GraphicsShader> asset) override;
		virtual Ref<GraphicsShader> LoadBinary(const Buffer& buffer) override;
		virtual void SaveBinary(Ref<GraphicsShader> asset) override;

	private:
		WeakRef<GraphicsContext> mContext;
	};
}