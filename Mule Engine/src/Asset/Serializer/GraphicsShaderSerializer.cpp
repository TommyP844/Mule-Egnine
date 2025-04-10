#include "Asset/Serializer/GraphicsShaderSerializer.h"

#include "Engine Context/EngineContext.h"

namespace Mule
{
	GraphicsShaderSerializer::GraphicsShaderSerializer(WeakRef<EngineContext> context)
		:
		mContext(context)
	{
	}

	Ref<GraphicsShader> GraphicsShaderSerializer::Load(const fs::path& filepath)
	{
		return mContext->GetGraphicsContext()->CreateGraphicsShader(filepath);
	}

	void GraphicsShaderSerializer::Save(Ref<GraphicsShader> asset)
	{
	}
}