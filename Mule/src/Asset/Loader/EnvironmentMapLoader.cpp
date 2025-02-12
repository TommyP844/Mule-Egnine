#include "Asset/Loader/EnvironmentMapLoader.h"

#include "Graphics/Execution/CommandBuffer.h"
#include "Graphics/Execution/CommandPool.h"
#include "Graphics/Execution/GraphicsQueue.h"
#include "Graphics/Texture/Texture2D.h"

#include <stb/stb_image.h>

// STD
#include <fstream>

namespace Mule
{
    EnvironmentMapLoader::EnvironmentMapLoader(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager)
        :
        mContext(context),
        mAssetManager(assetManager)
    {
        // Cube map descriptor
        {
            DescriptorSetLayoutDescription layoutDesc{};

            LayoutDescription layout{};

            layout.ArrayCount = 1;
            layout.Binding = 0;
            layout.Stage = ShaderStage::Compute;
            layout.Type = DescriptorType::Texture;
            layoutDesc.Layouts.push_back(layout);

            layout.ArrayCount = 1;
            layout.Binding = 1;
            layout.Stage = ShaderStage::Compute;
            layout.Type = DescriptorType::StorageImage;
            layoutDesc.Layouts.push_back(layout);

            mCubeMapDescriptorSetLayout = MakeRef<DescriptorSetLayout>(mContext, layoutDesc);

            DescriptorSetDescription descriptorDesc{};

            descriptorDesc.Layouts = { mCubeMapDescriptorSetLayout };

            mCubeMapDescriptorSet = MakeRef<DescriptorSet>(mContext, descriptorDesc);
        }

        // Cube map compute
        {
            ComputeShaderDescription computeDesc{};

            computeDesc.Filepath = "../Assets/Shaders/Compute/CubeMapCompute.glsl";
            computeDesc.Layouts = { mCubeMapDescriptorSetLayout };

            mCubeMapCompute = MakeRef<ComputeShader>(mContext, computeDesc);

            ComputeShaderDescription diffuseIBLDesc{};

            diffuseIBLDesc.Filepath = "../Assets/Shaders/Compute/DiffuseIBLCompute.glsl";
            diffuseIBLDesc.Layouts = { mCubeMapDescriptorSetLayout };

            mDiffuseIBLCompute = MakeRef<ComputeShader>(mContext, diffuseIBLDesc);

            ComputeShaderDescription preFilterDesc{};

            preFilterDesc.Filepath = "../Assets/Shaders/Compute/PrefilterEnvironmentMapCompute.glsl";
            preFilterDesc.Layouts = { mCubeMapDescriptorSetLayout };

            mPreFilterCompute = MakeRef<ComputeShader>(mContext, preFilterDesc);
        }

        // BRDF Descriptor set
        {
            DescriptorSetLayoutDescription layoutDesc{};

            LayoutDescription layout{};

            layout.ArrayCount = 1;
            layout.Binding = 0;
            layout.Stage = ShaderStage::Compute;
            layout.Type = DescriptorType::StorageImage;
            layoutDesc.Layouts.push_back(layout);

            mBRDFDescriptorSetLayout = MakeRef<DescriptorSetLayout>(mContext, layoutDesc);

            DescriptorSetDescription descriptorDesc{};

            descriptorDesc.Layouts = { mBRDFDescriptorSetLayout };

            mBRDFDescriptorSet = MakeRef<DescriptorSet>(mContext, descriptorDesc);
        }

        // BRDF Compute
        {
            ComputeShaderDescription computeDesc{};

            computeDesc.Filepath = "../Assets/Shaders/Compute/BRDFCompute.glsl";
            computeDesc.Layouts = { mBRDFDescriptorSetLayout };

            mBRDFCompute = MakeRef<ComputeShader>(mContext, computeDesc);

            Ref<Texture2D> brdfImage = MakeRef<Texture2D>(mContext, std::string("BRDF"), nullptr, 512, 512, TextureFormat::RGBA16F, TextureFlags::SotrageImage);

            auto queue = mContext->GetGraphicsQueue();
            auto fence = mContext->CreateFence();
            auto commandPool = queue->CreateCommandPool();
            auto commandBuffer = commandPool->CreateCommandBuffer();
            commandBuffer->Begin();
            commandBuffer->TranistionImageLayout(brdfImage, ImageLayout::General);

            DescriptorSetUpdate descUpdate{};
            descUpdate.ArrayElement = 0;
            descUpdate.Binding = 0;
            descUpdate.Type = DescriptorType::StorageImage;
            descUpdate.Textures = { brdfImage };

            mBRDFDescriptorSet->Update({ descUpdate });

            commandBuffer->BindComputeDescriptorSet(mBRDFCompute, mBRDFDescriptorSet);
            commandBuffer->BindComputePipeline(mBRDFCompute);
            commandBuffer->Execute(512 / 16, 512 / 16, 1);
            commandBuffer->TranistionImageLayout(brdfImage, ImageLayout::ShaderReadOnly);

            commandBuffer->End();
            fence->Wait();
            fence->Reset();
            queue->Submit(commandBuffer, {}, {}, fence);
            fence->Wait();

            mAssetManager->InsertAsset(brdfImage);
            mBRDFLutMap = brdfImage->Handle();
        }
    }

    Ref<EnvironmentMap> EnvironmentMapLoader::LoadText(const fs::path& filepath)
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

        Ref<TextureCube> cubeMap = MakeRef<TextureCube>(mContext, nullptr, 1024, 1, TextureFormat::RGBA32F, TextureFlags::SotrageImage);

        auto fence = mContext->CreateFence();
        auto queue = mContext->GetGraphicsQueue();
        auto commandPool = queue->CreateCommandPool();
        auto commandBuffer = commandPool->CreateCommandBuffer();
        commandBuffer->Begin();

        commandBuffer->TranistionImageLayout(cubeMap, ImageLayout::General);

        DescriptorSetUpdate update1{};
        update1.ArrayElement = 0;
        update1.Binding = 0;
        update1.Type = DescriptorType::Texture;
        update1.Textures = { texture };

        DescriptorSetUpdate update2{};
        update2.ArrayElement = 0;
        update2.Binding = 1;
        update2.Type = DescriptorType::StorageImage;
        update2.Textures = { cubeMap };

        mCubeMapDescriptorSet->Update({ update1, update2 });

        commandBuffer->BindComputeDescriptorSet(mCubeMapCompute, mCubeMapDescriptorSet);
        commandBuffer->BindComputePipeline(mCubeMapCompute);
        commandBuffer->Execute(1024 / 16, 1024 / 16, 6);

        commandBuffer->TranistionImageLayout(cubeMap, ImageLayout::ShaderReadOnly);

        commandBuffer->End();
        fence->Wait();
        fence->Reset();
        queue->Submit(commandBuffer, {}, {}, fence);
        fence->Wait();

        mAssetManager->InsertAsset(cubeMap);

        Ref<TextureCube> irradianceMap = MakeRef<TextureCube>(mContext, nullptr, 1024, 1, TextureFormat::RGBA16F, TextureFlags::SotrageImage);

        commandPool->Reset();
        commandBuffer->Begin();

        commandBuffer->TranistionImageLayout(irradianceMap, ImageLayout::General);

        update1.ArrayElement = 0;
        update1.Binding = 0;
        update1.Type = DescriptorType::Texture;
        update1.Textures = { cubeMap };

        update2.ArrayElement = 0;
        update2.Binding = 1;
        update2.Type = DescriptorType::StorageImage;
        update2.Textures = { irradianceMap };

        mCubeMapDescriptorSet->Update({ update1, update2 });

        commandBuffer->BindComputeDescriptorSet(mDiffuseIBLCompute, mCubeMapDescriptorSet);
        commandBuffer->BindComputePipeline(mDiffuseIBLCompute);
        commandBuffer->Execute(1024 / 8, 1024 / 8, 6);

        commandBuffer->TranistionImageLayout(irradianceMap, ImageLayout::ShaderReadOnly);

        commandBuffer->End();
        fence->Wait();
        fence->Reset();
        queue->Submit(commandBuffer, {}, {}, fence);
        fence->Wait();

        std::string filename = filepath.filename().replace_extension().string();
        irradianceMap->SetName(filename + "-IrradianceMap");
        mAssetManager->InsertAsset(irradianceMap);


        Ref<TextureCube> prefilterMap = MakeRef<TextureCube>(mContext, nullptr, 1024, 1, TextureFormat::RGBA16F, TextureFlags::SotrageImage);

        commandPool->Reset();
        commandBuffer->Begin();

        commandBuffer->TranistionImageLayout(prefilterMap, ImageLayout::General);

        update1.ArrayElement = 0;
        update1.Binding = 0;
        update1.Type = DescriptorType::Texture;
        update1.Textures = { cubeMap };

        update2.ArrayElement = 0;
        update2.Binding = 1;
        update2.Type = DescriptorType::StorageImage;
        update2.Textures = { prefilterMap };

        mCubeMapDescriptorSet->Update({ update1, update2 });

        commandBuffer->BindComputeDescriptorSet(mPreFilterCompute, mCubeMapDescriptorSet);
        commandBuffer->BindComputePipeline(mPreFilterCompute);
        commandBuffer->Execute(1024 / 8, 1024 / 8, 6);

        commandBuffer->TranistionImageLayout(prefilterMap, ImageLayout::ShaderReadOnly);

        commandBuffer->End();
        fence->Wait();
        fence->Reset();
        queue->Submit(commandBuffer, {}, {}, fence);
        fence->Wait();

        filename = filepath.filename().replace_extension().string();
        prefilterMap->SetName(filename + "-PreFilterMap");
        mAssetManager->InsertAsset(prefilterMap);



        return MakeRef<EnvironmentMap>(filepath, cubeMap->Handle(), mBRDFLutMap, irradianceMap->Handle(), prefilterMap->Handle());
    }

    void EnvironmentMapLoader::SaveText(Ref<EnvironmentMap> asset)
    {
    }

    Ref<EnvironmentMap> EnvironmentMapLoader::LoadBinary(const Buffer& buffer)
    {
        return nullptr;
    }

    void EnvironmentMapLoader::SaveBinary(Ref<EnvironmentMap> asset)
    {
    }
}
