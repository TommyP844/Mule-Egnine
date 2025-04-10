#include "Asset/Serializer/EnvironmentMapSerializer.h"

#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Execution/CommandPool.h"
#include "Graphics/Execution/GraphicsQueue.h"
#include "Graphics/Texture/Texture2D.h"
#include "Engine Context/EngineContext.h"

#include <stb/stb_image.h>

// STD
#include <fstream>

namespace Mule
{
    EnvironmentMapSerializer::EnvironmentMapSerializer(WeakRef<GraphicsContext> context, WeakRef<EngineContext> engineContext)
        :
        mContext(context),
        mEngineContext(engineContext)
    {
        // Cube map descriptor
        {
            mCubeMapDescriptorSetLayout = mContext->CreateDescriptorSetLayout({
                LayoutDescription(0, DescriptorType::Texture, ShaderStage::Compute, 1),
                LayoutDescription(1, DescriptorType::StorageImage, ShaderStage::Compute, 1)
                });

            mCubeMapDescriptorSet = mContext->CreateDescriptorSet({ mCubeMapDescriptorSetLayout });
        }

        // Cube map compute
        {
            mShaderLoadFuture = std::async(std::launch::async, [&]() {
                mCubeMapCompute = mContext->CreateComputeShader("../Assets/Shaders/Compute/CubeMapCompute.glsl");
                mDiffuseIBLCompute = mContext->CreateComputeShader("../Assets/Shaders/Compute/DiffuseIBLCompute.glsl");
                mPreFilterCompute = mContext->CreateComputeShader("../Assets/Shaders/Compute/PrefilterEnvironmentMapCompute.glsl");
                });
        }

        
        // BRDF Compute
        {
            int width, height, channels;
            void* data = stbi_load("../Assets/Textures/brdf_lut.png", &width, &height, &channels, STBI_rgb_alpha);
            if (data == nullptr)
            {
                SPDLOG_ERROR("Failed to load brdf image");
            }
            Ref<Texture2D> brdfImage = MakeRef<Texture2D>(mContext, std::string("BRDF"), data, width, height, TextureFormat::RGBA8U);


            mEngineContext->InsertAsset(brdfImage);
            mBRDFLutMap = brdfImage->Handle();
        }
    }

    Ref<EnvironmentMap> EnvironmentMapSerializer::Load(const fs::path& filepath)
    {
        if (!fs::exists(filepath))
            return nullptr;
        if (filepath.extension().string() != ".hdr")
            return nullptr;

        int width, height, channels;
        float* data = stbi_loadf(filepath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (data == nullptr)
            return nullptr;

        Ref<Texture2D> texture = MakeRef<Texture2D>(mContext, data, width, height, TextureFormat::RGBA32F);
        stbi_image_free(data);

        std::string filename = filepath.filename().replace_extension().string();

        auto fence = mContext->CreateFence();
        auto queue = mContext->GetBackgroundGraphicsQueue();
        auto commandPool = queue->CreateCommandPool();
        auto commandBuffer = commandPool->CreateCommandBuffer();

        DescriptorSetUpdate update1{};
        DescriptorSetUpdate update2{};

        mShaderLoadFuture.wait();
        std::lock_guard<std::mutex> lock(mMutex);

        // Cube Map
        Ref<TextureCube> cubeMap = nullptr;
        {
            cubeMap = MakeRef<TextureCube>(mContext, nullptr, 1024, 1, TextureFormat::RGBA32F, (TextureFlags)(TextureFlags::StorageImage | TextureFlags::GenerateMips));

            commandBuffer->Begin();

            commandBuffer->TranistionImageLayout(cubeMap, ImageLayout::General);
            
            DescriptorSetUpdate update1(0, DescriptorType::Texture, 0, { texture }, {});
            DescriptorSetUpdate update2(1, DescriptorType::StorageImage, 0, { cubeMap }, {});

            mCubeMapDescriptorSet->Update({ update1, update2 });

            commandBuffer->BindComputeDescriptorSet(mCubeMapCompute, mCubeMapDescriptorSet);
            commandBuffer->BindComputePipeline(mCubeMapCompute);
            commandBuffer->Execute((1024 + 32) / 32, (1024 + 32) / 32, 6);

            commandBuffer->TranistionImageLayout(cubeMap, ImageLayout::ShaderReadOnly);

            commandBuffer->End();
            fence->Wait();
            fence->Reset();
            queue->Submit(commandBuffer, {}, {}, fence);
            fence->Wait();

            cubeMap->GenerateMips();
            cubeMap->SetName(filename);
            mEngineContext->InsertAsset(cubeMap);
        }
    

        Ref<TextureCube> irradianceMap;
        // Irradiance Map
        {
            irradianceMap = MakeRef<TextureCube>(mContext, nullptr, 1024, 1, TextureFormat::RGBA16F, TextureFlags::StorageImage);

            commandPool->Reset();
            commandBuffer->Begin();

            commandBuffer->TranistionImageLayout(irradianceMap, ImageLayout::General);

            DescriptorSetUpdate update1(0, DescriptorType::Texture, 0, { cubeMap }, {});
            DescriptorSetUpdate update2(1, DescriptorType::StorageImage, 0, { irradianceMap }, {});

            mCubeMapDescriptorSet->Update({ update1, update2 });

            commandBuffer->BindComputeDescriptorSet(mDiffuseIBLCompute, mCubeMapDescriptorSet);
            commandBuffer->BindComputePipeline(mDiffuseIBLCompute);
            commandBuffer->Execute((1024 + 32) / 32, (1024 + 32) / 32, 6);

            commandBuffer->TranistionImageLayout(irradianceMap, ImageLayout::ShaderReadOnly);

            commandBuffer->End();
            fence->Wait();
            fence->Reset();
            queue->Submit(commandBuffer, {}, {}, fence);
            fence->Wait();

            irradianceMap->SetName(filename + "-IrradianceMap");
            mEngineContext->InsertAsset(irradianceMap);
        }

        Ref<TextureCube> prefilterMap = nullptr;
        // Pre-Filter Map
        {
            prefilterMap = MakeRef<TextureCube>(mContext, nullptr, 1024, 1, TextureFormat::RGBA16F, (TextureFlags)(TextureFlags::StorageImage | TextureFlags::GenerateMips));

            uint32_t preFilterMipLevelCount = prefilterMap->GetMipCount();
            for (uint32_t i = 0; i < preFilterMipLevelCount; i++)
            {
                commandPool->Reset();
                commandBuffer->Begin();

                commandBuffer->TranistionImageLayout(prefilterMap, ImageLayout::General);
                                
                DescriptorSetUpdate update(1, DescriptorType::StorageImage, 0, prefilterMap->GetMipImageView(i), prefilterMap->GetSampler(), prefilterMap->GetVulkanImage().Layout);

                mCubeMapDescriptorSet->Update({ update });

                commandBuffer->BindComputeDescriptorSet(mPreFilterCompute, mCubeMapDescriptorSet);
                commandBuffer->BindComputePipeline(mPreFilterCompute);

                float roughness = (static_cast<float>(i)) / (static_cast<float>(preFilterMipLevelCount - 1.f));

                glm::ivec2 dimension = prefilterMap->GetMipLevelSize(i);
                commandBuffer->SetPushConstants(mPreFilterCompute, &roughness, sizeof(float));
                commandBuffer->Execute((dimension.x + 32) / 32, (dimension.y + 32) / 32, 6);
                commandBuffer->TranistionImageLayout(prefilterMap, ImageLayout::ShaderReadOnly);

                commandBuffer->End();
                fence->Wait();
                fence->Reset();
                queue->Submit(commandBuffer, {}, {}, fence);
                fence->Wait();
            }

            filename = filepath.filename().replace_extension().string();
            prefilterMap->SetName(filename + "-PreFilterMap");
            mEngineContext->InsertAsset(prefilterMap);
        }

        return MakeRef<EnvironmentMap>(filepath, cubeMap->Handle(), mBRDFLutMap, irradianceMap->Handle(), prefilterMap->Handle());
    }

    void EnvironmentMapSerializer::Save(Ref<EnvironmentMap> asset)
    {
    }
}
