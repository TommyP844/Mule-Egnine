#include "Asset/Loader/EnvironmentMapLoader.h"

namespace Mule
{
    EnvironmentMapLoader::EnvironmentMapLoader(WeakRef<GraphicsContext> context, WeakRef<AssetManager> assetManager)
        :
        mContext(context),
        mAssetManager(assetManager)
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

        mDescriptorSetLayout = MakeRef<DescriptorSetLayout>(context, layoutDesc);

        ComputeShaderDescription computeDesc{};

        computeDesc.Filepath = "../Assets/Shaders/Compute/CubeMapCompute.glsl";
        computeDesc.Layouts = { mDescriptorSetLayout };

        mCubeMapCompute = MakeRef<ComputeShader>(context, computeDesc);
    }

    Ref<EnvironmentMap> EnvironmentMapLoader::LoadText(const fs::path& filepath)
    {
        return nullptr;
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
