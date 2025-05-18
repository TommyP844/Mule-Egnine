#include "Asset/Generator/EnvironmentMapGenerator.h"
#include "Engine Context/EngineAssets.h"

#include "Asset/AssetManager.h"	
#include "Graphics/ShaderFactory.h"
#include "Graphics/API/TextureCube.h"

namespace Mule
{
	EnvironmentMapGenerator::EnvironmentMapGenerator(Ref<ServiceManager> serviceManager)
		:
		IAssetGenerator(serviceManager)
	{
	}
	
	Ref<EnvironmentMap> EnvironmentMapGenerator::Generate(const std::filesystem::path& outFilepath, AssetHandle hdrImageHandle, uint32_t size)
	{

		auto assetManager = mServiceManager->Get<AssetManager>();

		auto shaderFactory = ShaderFactory::Get();
		auto hdrToCubemapShader = shaderFactory.GetOrCreateComputePipeline("HDRToCubemap");
		auto diffuseIBLShader = shaderFactory.GetOrCreateComputePipeline("DiffuseIBL");
		auto prefilterIBLShader = shaderFactory.GetOrCreateComputePipeline("PrefilterIBL");

		Ref<ShaderResourceGroup> hdrToCubemapSRG = ShaderResourceGroup::Create(hdrToCubemapShader->GetBlueprintIndex(0));
		Ref<ShaderResourceGroup> diffuseIBLSRG = ShaderResourceGroup::Create(diffuseIBLShader->GetBlueprintIndex(0));
		Ref<ShaderResourceGroup> prefilterIBLSRG = ShaderResourceGroup::Create(prefilterIBLShader->GetBlueprintIndex(0));

		Ref<GraphicsQueue> queue = GraphicsQueue::Create();
		Ref<CommandAllocator> commandAllocator = CommandAllocator::Create();
		Ref<CommandBuffer> commandBuffer = commandAllocator->CreateCommandBuffer();
		Ref<Fence> fence = Fence::Create();
		fence->Reset();

		// Convert 2D Texture to Cube map
		
		Ref<TextureCube> cubeMap = TextureCube::Create("Environment Map", {}, size, TextureFormat::RGBA_16F, TextureFlags::TransferSrc | TextureFlags::TransferDst | TextureFlags::StorageImage);
		cubeMap->TransitionImageLayoutImmediate(ImageLayout::General);

		auto hdrImage = assetManager->Get<Texture>(hdrImageHandle);
		hdrToCubemapSRG->Update(0, DescriptorType::Texture, ImageLayout::ShaderReadOnly, hdrImage);
		hdrToCubemapSRG->Update(1, DescriptorType::StorageImage, ImageLayout::General, (WeakRef<Texture>)cubeMap);

		commandBuffer->Begin();
		commandBuffer->BindComputePipeline(hdrToCubemapShader, { hdrToCubemapSRG });
		commandBuffer->Execute((size + 32) / 32, (size + 32) / 32, 6);
		commandBuffer->TranistionImageLayout(cubeMap, ImageLayout::ShaderReadOnly);
		commandBuffer->End();

		queue->Submit(commandBuffer, {}, {}, fence);
		fence->Wait();
		fence->Reset();
		assetManager->Insert(cubeMap);
		

		// Generate irradiance map
		Ref<TextureCube> diffuseIBL = TextureCube::Create("Diffuse IBL", {}, size, TextureFormat::RGBA_16F, TextureFlags::TransferSrc | TextureFlags::TransferDst | TextureFlags::StorageImage);
		diffuseIBL->TransitionImageLayoutImmediate(ImageLayout::General);
		
		diffuseIBLSRG->Update(0, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)cubeMap);
		diffuseIBLSRG->Update(1, DescriptorType::StorageImage, ImageLayout::General, (WeakRef<Texture>)diffuseIBL);

		commandBuffer->Reset();
		commandBuffer->Begin();
		commandBuffer->BindComputePipeline(diffuseIBLShader, { diffuseIBLSRG });
		commandBuffer->Execute((size + 32) / 32, (size + 32) / 32, 6);
		commandBuffer->TranistionImageLayout(diffuseIBL, ImageLayout::ShaderReadOnly);
		commandBuffer->End();

		queue->Submit(commandBuffer, {}, {}, fence);
		fence->Wait();
		fence->Reset();
		assetManager->Insert(diffuseIBL);
		

		// Generate pre-filtered map
		
		Ref<TextureCube> prefilterMap = TextureCube::Create("Prefiltered IBL", {}, size, TextureFormat::RGBA_16F, TextureFlags::TransferSrc | TextureFlags::TransferDst | TextureFlags::StorageImage | TextureFlags::GenerateMips);
		prefilterMap->TransitionImageLayoutImmediate(ImageLayout::General);

		uint32_t mipLevels = prefilterMap->GetMipLevels();
		for (uint32_t i = 0; i < mipLevels; i++)
		{
			float roughness = (float)i / (mipLevels - 1);

			commandBuffer->Reset();
			commandBuffer->Begin();

			auto view = prefilterMap->GetMipView(i);

			prefilterIBLSRG->Update(0, DescriptorType::Texture, ImageLayout::ShaderReadOnly, (WeakRef<Texture>)cubeMap);
			prefilterIBLSRG->Update(1, DescriptorType::StorageImage, ImageLayout::General, view);

			commandBuffer->BindComputePipeline(prefilterIBLShader, { prefilterIBLSRG });
			commandBuffer->SetPushConstants(prefilterIBLShader, &roughness, sizeof(roughness));
			commandBuffer->Execute((size + 32) / 32, (size + 32) / 32, 6);

			if (i == mipLevels - 1)
			{
				commandBuffer->TranistionImageLayout(prefilterMap, ImageLayout::ShaderReadOnly);
			}

			commandBuffer->End();
			queue->Submit(commandBuffer, {}, {}, fence);
			fence->Wait();
			fence->Reset();
		}
		assetManager->Insert(prefilterMap);
		

		auto envMap = MakeRef<EnvironmentMap>(outFilepath, cubeMap->Handle(), diffuseIBL->Handle(), prefilterMap->Handle());
		assetManager->Insert(envMap);

		return envMap;
	}

	Ref<EnvironmentMap> EnvironmentMapGenerator::Generate(const std::filesystem::path& outFilepath, const std::vector<AssetHandle>& axisImages, uint32_t size)
	{
		return Ref<EnvironmentMap>();
	}
}