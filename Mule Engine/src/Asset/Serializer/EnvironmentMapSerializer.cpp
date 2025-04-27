#include "Asset/Serializer/EnvironmentMapSerializer.h"

#include "Graphics/API/CommandBuffer.h"
#include "Graphics/API/CommandAllocator.h"
#include "Graphics/API/GraphicsQueue.h"
#include "Graphics/API/Texture2D.h"
#include "Graphics/API/ShaderResourceBlueprint.h"
#include "Graphics/API/ShaderResourceGroup.h"
#include "Graphics/API/Image.h"

#include <stb/stb_image.h>

// STD
#include <fstream>

namespace Mule
{
    EnvironmentMapSerializer::EnvironmentMapSerializer(Ref<ServiceManager> serviceManager)
        :
        IAssetSerializer(serviceManager)
    {
        auto assetManager = mServiceManager->Get<AssetManager>();

        // Cube map descriptor
        {
            mCubeMapDescriptorSetLayout = ShaderResourceBlueprint::Create({
                ShaderResourceDescription(0, ShaderResourceType::Sampler, ShaderStage::Compute, 1),
                ShaderResourceDescription(1, ShaderResourceType::StorageImage, ShaderStage::Compute, 1)
                });

            mCubeMapDescriptorSet = ShaderResourceGroup::Create({ mCubeMapDescriptorSetLayout });
        }

        // Cube map compute
        {
            mShaderLoadFuture = std::async(std::launch::async, [&]() {
                //mCubeMapCompute = gContext->CreateComputeShader("../Assets/Shaders/Compute/CubeMapCompute.glsl");
                //mDiffuseIBLCompute = gContext->CreateComputeShader("../Assets/Shaders/Compute/DiffuseIBLCompute.glsl");
                //mPreFilterCompute = gContext->CreateComputeShader("../Assets/Shaders/Compute/PrefilterEnvironmentMapCompute.glsl");
                });
        }

        
        // BRDF Compute
        {
            Image image = Image::Load("../Assets/Textures/brdf_lut.png");
 
            Ref<Texture2D> brdfImage = Texture2D::Create("BRDF", image.Data, image.Width, image.Height, image.Format, TextureFlags::TransferDst);

            assetManager->InsertAsset(brdfImage);
            mBRDFLutMap = brdfImage->Handle();
        }
    }

    Ref<EnvironmentMap> EnvironmentMapSerializer::Load(const fs::path& filepath)
    {
        return nullptr;

        if (!fs::exists(filepath))
            return nullptr;
        if (filepath.extension().string() != ".hdr")
            return nullptr;

        auto graphicsContext = mServiceManager->Get<GraphicsContext>();
        auto assetManager = mServiceManager->Get<AssetManager>();

        Image image = Image::Load(filepath);

        Ref<Texture2D> texture = Texture2D::Create("", image.Data, image.Width, image.Height, TextureFormat::RGBA_32F, TextureFlags::TransferDst);
        image.Data.Release();

        std::string filename = filepath.filename().replace_extension().string();

        auto fence = Fence::Create();
        auto commandPool = CommandAllocator::Create();
        auto commandBuffer = commandPool->CreateCommandBuffer();

        //DescriptorSetUpdate update1{};
        //DescriptorSetUpdate update2{};

        mShaderLoadFuture.wait();
        std::lock_guard<std::mutex> lock(mMutex);

        // Cube Map
        Ref<TextureCube> cubeMap = nullptr;
        {
            cubeMap = TextureCube::Create("", Buffer(), 1024, TextureFormat::RGBA_32F, TextureFlags::StorageImage | TextureFlags::GenerateMips);

            commandBuffer->Begin();

            commandBuffer->TranistionImageLayout(cubeMap, ImageLayout::General);
            
            //DescriptorSetUpdate update1(0, DescriptorType::Texture, 0, { texture }, {});
            //DescriptorSetUpdate update2(1, DescriptorType::StorageImage, 0, { cubeMap }, {});
            //
            //mCubeMapDescriptorSet->Update({ update1, update2 });
            //
            //commandBuffer->BindComputeDescriptorSet(mCubeMapCompute, mCubeMapDescriptorSet);
            //commandBuffer->BindComputePipeline(mCubeMapCompute);
            commandBuffer->Execute((1024 + 32) / 32, (1024 + 32) / 32, 6);

            commandBuffer->TranistionImageLayout(cubeMap, ImageLayout::ShaderReadOnly);

            commandBuffer->End();
            fence->Wait();
            fence->Reset();
            //queue->Submit(commandBuffer, {}, {}, fence);
            fence->Wait();

            //cubeMap->GenerateMips();
            //cubeMap->SetName(filename);
            assetManager->InsertAsset(cubeMap);
        }
    

        Ref<TextureCube> irradianceMap;
        // Irradiance Map
        {
            irradianceMap = TextureCube::Create("", {}, 1024, TextureFormat::RGBA_16F, TextureFlags::StorageImage);

            commandPool->Reset();
            commandBuffer->Begin();

            commandBuffer->TranistionImageLayout(irradianceMap, ImageLayout::General);

            //DescriptorSetUpdate update1(0, DescriptorType::Texture, 0, { cubeMap }, {});
            //DescriptorSetUpdate update2(1, DescriptorType::StorageImage, 0, { irradianceMap }, {});

            //mCubeMapDescriptorSet->Update({ update1, update2 });

            //commandBuffer->BindComputeDescriptorSet(mDiffuseIBLCompute, mCubeMapDescriptorSet);
            //commandBuffer->BindComputePipeline(mDiffuseIBLCompute);
            commandBuffer->Execute((1024 + 32) / 32, (1024 + 32) / 32, 6);

            commandBuffer->TranistionImageLayout(irradianceMap, ImageLayout::ShaderReadOnly);

            commandBuffer->End();
            fence->Wait();
            fence->Reset();
            //queue->Submit(commandBuffer, {}, {}, fence);
            fence->Wait();

            //irradianceMap->SetName(filename + "-IrradianceMap");
            assetManager->InsertAsset(irradianceMap);
        }

        Ref<TextureCube> prefilterMap = nullptr;
        // Pre-Filter Map
        {
            prefilterMap = TextureCube::Create("", {}, 1024, TextureFormat::RGBA_16F, (TextureFlags)(TextureFlags::StorageImage | TextureFlags::GenerateMips));

            //uint32_t preFilterMipLevelCount = prefilterMap->GetMipCount();
            //for (uint32_t i = 0; i < preFilterMipLevelCount; i++)
            //{
            //    commandPool->Reset();
            //    commandBuffer->Begin();
            //
            //    commandBuffer->TranistionImageLayout(prefilterMap, ImageLayout::General);
            //                    
            //    //DescriptorSetUpdate update(1, DescriptorType::StorageImage, 0, prefilterMap->GetMipImageView(i), prefilterMap->GetSampler(), prefilterMap->GetVulkanImage().Layout);
            //
            //    //mCubeMapDescriptorSet->Update({ update });
            //
            //    //commandBuffer->BindComputeDescriptorSet(mPreFilterCompute, mCubeMapDescriptorSet);
            //   //commandBuffer->BindComputePipeline(mPreFilterCompute);
            //
            //    float roughness = (static_cast<float>(i)) / (static_cast<float>(preFilterMipLevelCount - 1.f));
            //
            //    //glm::ivec2 dimension = prefilterMap->GetMipLevelSize(i);
            //    //commandBuffer->SetPushConstants(mPreFilterCompute, &roughness, sizeof(float));
            //    //commandBuffer->Execute((dimension.x + 32) / 32, (dimension.y + 32) / 32, 6);
            //    //commandBuffer->TranistionImageLayout(prefilterMap, ImageLayout::ShaderReadOnly);
            //
            //    commandBuffer->End();
            //    fence->Wait();
            //    fence->Reset();
            //    //queue->Submit(commandBuffer, {}, {}, fence);
            //    fence->Wait();
            //}
            //
            //filename = filepath.filename().replace_extension().string();
            ////prefilterMap->SetName(filename + "-PreFilterMap");
            //assetManager->InsertAsset(prefilterMap);
        }

        return nullptr; //MakeRef<EnvironmentMap>(filepath, cubeMap->Handle(), mBRDFLutMap, irradianceMap->Handle(), prefilterMap->Handle());
    }

    void EnvironmentMapSerializer::Save(Ref<EnvironmentMap> asset)
    {
    }
}
