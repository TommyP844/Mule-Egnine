#include "Asset/Serializer/GraphicsShaderSerializer.h"

#include "Engine Context/EngineContext.h"

namespace Mule
{
	GraphicsShaderSerializer::GraphicsShaderSerializer(WeakRef<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	Ref<GraphicsShader> GraphicsShaderSerializer::Load(const fs::path& filepath)
	{
		auto graphicsContext = mServiceManager->Get<GraphicsContext>();

		return graphicsContext->CreateGraphicsShader(filepath);
	}

	void GraphicsShaderSerializer::Save(Ref<GraphicsShader> asset)
	{
	}
}