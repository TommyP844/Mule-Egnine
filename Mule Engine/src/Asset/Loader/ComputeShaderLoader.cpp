#include "Asset/Loader/ComputeShaderLoader.h"

#include "Engine Context/EngineContext.h"

namespace Mule
{
	ComputeShaderLoader::ComputeShaderLoader(WeakRef<EngineContext> context)
		:
		mContext(context)
	{
	}

	ComputeShaderLoader::~ComputeShaderLoader()
	{
	}

	Ref<ComputeShader> ComputeShaderLoader::LoadText(const fs::path& filepath)
	{
		return mContext->GetGraphicsContext()->CreateComputeShader(filepath);
	}

	void ComputeShaderLoader::SaveText(Ref<ComputeShader> asset)
	{
	}

	Ref<ComputeShader> ComputeShaderLoader::LoadBinary(const Buffer& buffer)
	{
		return Ref<ComputeShader>();
	}

	void ComputeShaderLoader::SaveBinary(Ref<ComputeShader> asset)
	{
	}
}