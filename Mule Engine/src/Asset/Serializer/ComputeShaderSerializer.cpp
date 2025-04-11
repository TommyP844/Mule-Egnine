#include "Asset/Serializer/ComputeShaderSerializer.h"

#include "Engine Context/EngineContext.h"

namespace Mule
{
	ComputeShaderSerializer::ComputeShaderSerializer(WeakRef<ServiceManager> serviceManager)
		:
		IAssetSerializer(serviceManager)
	{
	}

	ComputeShaderSerializer::~ComputeShaderSerializer()
	{
	}

	Ref<ComputeShader> ComputeShaderSerializer::Load(const fs::path& filepath)
	{
		auto graphicsContext = mServiceManager->Get<GraphicsContext>();

		return graphicsContext->CreateComputeShader(filepath);
	}

	void ComputeShaderSerializer::Save(Ref<ComputeShader> asset)
	{
	}
}