#include "Asset/Serializer/ComputeShaderSerializer.h"

#include "Engine Context/EngineContext.h"

namespace Mule
{
	ComputeShaderSerializer::ComputeShaderSerializer(WeakRef<EngineContext> context)
		:
		mContext(context)
	{
	}

	ComputeShaderSerializer::~ComputeShaderSerializer()
	{
	}

	Ref<ComputeShader> ComputeShaderSerializer::Load(const fs::path& filepath)
	{
		return mContext->GetGraphicsContext()->CreateComputeShader(filepath);
	}

	void ComputeShaderSerializer::Save(Ref<ComputeShader> asset)
	{
	}
}