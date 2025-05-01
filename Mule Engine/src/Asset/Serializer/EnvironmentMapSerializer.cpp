#include "Asset/Serializer/EnvironmentMapSerializer.h"

// STD
#include <fstream>
#include <yaml-cpp/yaml.h>

#include "FileIO.h"

// TODO: Move this file to a new folder under mule engine
#include "Asset/Serializer/Convert/YamlConvert.h"

namespace Mule
{
    EnvironmentMapSerializer::EnvironmentMapSerializer(Ref<ServiceManager> serviceManager)
        :
        IAssetSerializer(serviceManager)
    {
    }

    Ref<EnvironmentMap> EnvironmentMapSerializer::Load(const fs::path& filepath)
    {
		YAML::Node node = YAML::LoadFile(filepath.string());

        auto cubeMapNode = node["CubeMap"];
        TextureFormat format = cubeMapNode["Format"].as<TextureFormat>();
        uint32_t axisSize = cubeMapNode["AxisSize"].as<uint32_t>();
        AssetHandle handle = cubeMapNode["Handle"].as<AssetHandle>();

        std::string filename = filepath.filename().replace_extension().string();
		fs::path path = filepath.parent_path();

        Buffer buffer;
		fs::path bufferPath = path / (std::to_string((uint64_t)handle) + ".bin");
        if (!ReadFileBytes(bufferPath, buffer))
        {
			SPDLOG_ERROR("Failed to read file: {}", bufferPath.string());
            return nullptr;
        }
        Ref<TextureCube> cubeMap = TextureCube::Create(filename + "-CubeMap", buffer, axisSize, format, TextureFlags::TransferDst);
		cubeMap->SetHandle(handle);
		buffer.Release();

        auto diffuseMapNode = node["DiffuseIBL"];
        format = diffuseMapNode["Format"].as<TextureFormat>();
        axisSize = diffuseMapNode["AxisSize"].as<uint32_t>();
        handle = diffuseMapNode["Handle"].as<AssetHandle>();

        bufferPath = path / (std::to_string((uint64_t)handle) + ".bin");
        if (!ReadFileBytes(bufferPath, buffer))
        {
            SPDLOG_ERROR("Failed to read file: {}", bufferPath.string());
            return nullptr;
        }
        Ref<TextureCube> diffuseIBLMap = TextureCube::Create(filename + "-DiffuseIBL", buffer, axisSize, format, TextureFlags::TransferDst);
		diffuseIBLMap->SetHandle(handle);
        buffer.Release();

        auto preFilterMapNode = node["PreFilterMap"];
        format = preFilterMapNode["Format"].as<TextureFormat>();
        axisSize = preFilterMapNode["AxisSize"].as<uint32_t>();
        handle = preFilterMapNode["Handle"].as<AssetHandle>();
		uint32_t mipLevels = preFilterMapNode["MipLevels"].as<uint32_t>();

        bufferPath = path / (std::to_string((uint64_t)handle) + ".bin");
        if (!ReadFileBytes(bufferPath, buffer))
        {
            SPDLOG_ERROR("Failed to read file: {}", bufferPath.string());
            return nullptr;
        }
        Ref<TextureCube> prefilterIBLMap = TextureCube::Create(filename + "-Prefilter", buffer, axisSize, format, TextureFlags::TransferDst | TextureFlags::GenerateMips);
		prefilterIBLMap->SetHandle(handle);

        uint32_t offset = 0;
        for (uint32_t i = 0; i < mipLevels; i++)
        {
			uint32_t mipWidth = std::max(axisSize >> i, 1u);
			uint32_t size = mipWidth * mipWidth * 6 * GetFormatSize(format);

			uint8_t* data = buffer.As<uint8_t>() + offset;
			Buffer mipBuffer = Buffer(data, size);
			offset += size;
            prefilterIBLMap->WriteMipLevel(i, mipBuffer);
        }

        buffer.Release();

		auto assetManager = mServiceManager->Get<AssetManager>();
		assetManager->InsertAsset(cubeMap);
		assetManager->InsertAsset(diffuseIBLMap);
		assetManager->InsertAsset(prefilterIBLMap);

        return MakeRef<EnvironmentMap>(filepath, cubeMap->Handle(), diffuseIBLMap->Handle(), prefilterIBLMap->Handle());

    }

    void EnvironmentMapSerializer::Save(Ref<EnvironmentMap> asset)
    {
		auto assetManager = mServiceManager->Get<AssetManager>();

		auto cubeMap = assetManager->GetAsset<Texture>(asset->GetCubeMapHandle());
		auto diffuseMap = assetManager->GetAsset<Texture>(asset->GetDiffuseIBLMap());
		auto preFilterMap = assetManager->GetAsset<Texture>(asset->GetPreFilterMap());

		Buffer cubeMapData = cubeMap->ReadTextureData();
		Buffer diffuseMapData = diffuseMap->ReadTextureData();

        auto path = asset->FilePath().parent_path();

        // Cube Map
        {
            auto filepath = path / (std::to_string((uint64_t)asset->GetCubeMapHandle()) + ".bin");
            auto file = std::ofstream(filepath, std::ios::binary);
            if (!file)
            {
                SPDLOG_ERROR("Failed to open file for writing: {}", filepath.string());
                return;
            }

            file.write(reinterpret_cast<const char*>(cubeMapData.GetData()), cubeMapData.GetSize());
            cubeMapData.Release();
            file.close();
        }

        // Diffuse IBL
        {
            auto filepath = path / (std::to_string((uint64_t)asset->GetDiffuseIBLMap()) + ".bin");
            auto file = std::ofstream(filepath, std::ios::binary);
            if (!file)
            {
                SPDLOG_ERROR("Failed to open file for writing: {}", filepath.string());
                return;
            }

            file.write(reinterpret_cast<const char*>(diffuseMapData.GetData()), diffuseMapData.GetSize());
            diffuseMapData.Release();
            file.close();
        }
        
        // Prefilter IBL
        {
            auto filepath = path / (std::to_string((uint64_t)asset->GetPreFilterMap()) + ".bin");
            auto file = std::ofstream(filepath, std::ios::binary);
            if (!file)
            {
                SPDLOG_ERROR("Failed to open file for writing: {}", filepath.string());
                return;
            }

            for (uint32_t i = 0; i < preFilterMap->GetMipLevels(); i++)
            {
                Buffer preFilterMapData = preFilterMap->ReadTextureData(i);
                file.write(reinterpret_cast<const char*>(preFilterMapData.GetData()), preFilterMapData.GetSize());
                preFilterMapData.Release();
            }
            file.close();
        }

        YAML::Emitter emitter;
        YAML::Node node;

        auto cubeMapNode = node["CubeMap"];
        cubeMapNode["Format"] = cubeMap->GetFormat();
		cubeMapNode["AxisSize"] = cubeMap->GetWidth();
        cubeMapNode["Handle"] = cubeMap->Handle();

		auto diffuseMapNode = node["DiffuseIBL"];
		diffuseMapNode["Format"] = diffuseMap->GetFormat();
		diffuseMapNode["AxisSize"] = diffuseMap->GetWidth();
		diffuseMapNode["Handle"] = diffuseMap->Handle();

		auto preFilterMapNode = node["PreFilterMap"];
		preFilterMapNode["Format"] = preFilterMap->GetFormat();
		preFilterMapNode["AxisSize"] = preFilterMap->GetWidth();
		preFilterMapNode["MipLevels"] = preFilterMap->GetMipLevels();
		preFilterMapNode["Handle"] = preFilterMap->Handle();

        emitter << node;

        std::ofstream file(asset->FilePath());
		if (!file)
		{
			SPDLOG_ERROR("Failed to open file for writing: {}", asset->FilePath().string());
			return;
		}

        file << emitter.c_str();
        file.close();
    }
}
