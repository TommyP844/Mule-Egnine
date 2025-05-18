#pragma once

#include "Ref.h"

#include "Graphics/Renderer/RenderGraph/ResourceHandle.h"

// Resources
#include "Graphics/API/ShaderResourceBlueprint.h"
#include "Graphics/Renderer/RenderGraph/ResourceType.h"
#include "Graphics/API/Sampler.h"

#include <unordered_map>
#include <variant>

namespace Mule
{
	class ResourceBuilder
	{
	public:
		ResourceBuilder();
		~ResourceBuilder() = default;

		ResourceHandle CreateSampler(const std::string& name, const SamplerDescription& description);
		ResourceHandle CreateUniformBuffer(const std::string& name, uint32_t bufferSize);
		ResourceHandle CreateTexture2D(const std::string& name, TextureFormat format, TextureFlags flags);
		ResourceHandle CreateTexture2DArray(const std::string& name, uint32_t layers, TextureFormat format, TextureFlags flags);
		ResourceHandle CreateSRG(const std::string& name, const std::vector<ShaderResourceDescription>& resources);

		struct SamplerBlueprint { SamplerDescription Description; };
		struct UniformBufferBlueprint { uint32_t Size; };
		struct SRGBlueprint { std::vector<ShaderResourceDescription> Descriptions; };
		struct Texture2DBlueprint { TextureFormat format; TextureFlags flags; ResourceType Type; };
		struct Texture2DArrayBlueprint { uint32_t Layers; TextureFormat Format; TextureFlags Flags; ResourceType Type; };

		const std::unordered_map<std::string, SamplerBlueprint>& GetSamplerBlueprints() const { return mSamplerBlueprints; }
		const std::unordered_map<std::string, UniformBufferBlueprint>& GetUniformBufferBlueprints() const { return mUniformBufferBlueprints; }
		const std::unordered_map<std::string, SRGBlueprint>& GetSRGBlueprints() const { return mSRGBlueprints; }
		const std::unordered_map<std::string, Texture2DBlueprint>& GetTextureBlueprints() const { return mTexture2DBlueprints; }
		const std::unordered_map<std::string, Texture2DArrayBlueprint>& GetTexture2DArrayBlueprints() const { return mTexture2DArrayBlueprints; }
	private:

		template<typename T>
		using ResourceBlueprintMap = std::unordered_map<std::string, T>;

		ResourceBlueprintMap<SamplerBlueprint> mSamplerBlueprints;
		ResourceBlueprintMap<UniformBufferBlueprint> mUniformBufferBlueprints;
		ResourceBlueprintMap<SRGBlueprint> mSRGBlueprints;
		ResourceBlueprintMap<Texture2DBlueprint> mTexture2DBlueprints;
		ResourceBlueprintMap<Texture2DArrayBlueprint> mTexture2DArrayBlueprints;

		ResourceHandle mOutputFBHandle;
	};
}