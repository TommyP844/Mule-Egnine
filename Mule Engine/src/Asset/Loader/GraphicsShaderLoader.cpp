#include "Asset/Loader/GraphicsShaderLoader.h"

#include "Engine Context/EngineContext.h"

namespace Mule
{
	GraphicsShaderLoader::GraphicsShaderLoader(WeakRef<EngineContext> context)
		:
		mContext(context)
	{
	}

	Ref<GraphicsShader> GraphicsShaderLoader::LoadText(const fs::path& filepath)
	{
		return mContext->GetGraphicsContext()->CreateGraphicsShader(filepath);
	}

	void GraphicsShaderLoader::SaveText(Ref<GraphicsShader> asset)
	{
	}

	Ref<GraphicsShader> GraphicsShaderLoader::LoadBinary(const Buffer& buffer)
	{
		return nullptr;
	}

	void GraphicsShaderLoader::SaveBinary(Ref<GraphicsShader> asset)
	{
	}
}