#include "Asset/Loader/GraphicsShaderLoader.h"


namespace Mule
{
	GraphicsShaderLoader::GraphicsShaderLoader(WeakRef<GraphicsContext> context)
		:
		mContext(context)
	{
	}

	Ref<GraphicsShader> GraphicsShaderLoader::LoadText(const fs::path& filepath)
	{
		return nullptr;
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