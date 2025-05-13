#pragma once

#include "Ref.h"

#include "Graphics/Renderer/RenderGraph/ResourceHandle.h"

// Resources
#include "Graphics/API/ShaderResourceBlueprint.h"
#include "Graphics/Renderer/RenderGraph/ResourceType.h"

#include <unordered_map>
#include <variant>

namespace Mule
{
	class ResourceBuilder
	{
	public:
		ResourceBuilder();
		~ResourceBuilder() = default;

		ResourceHandle CreateUniformBuffer(const std::string& name, uint32_t bufferSize);
		ResourceHandle CreateTexture2D(const std::string& name, TextureFormat format, TextureFlags flags);
		ResourceHandle CreateSRG(const std::string& name, const std::vector<ShaderResourceDescription>& resources);

		struct UniformBufferBlueprint { uint32_t Size; };
		struct SRGBlueprint { std::vector<ShaderResourceDescription> Descriptions; };
		struct Texture2DBlueprint { TextureFormat format; TextureFlags flags; ResourceType Type; };

		const std::unordered_map<std::string, UniformBufferBlueprint>& GetUniformBufferBlueprints() const { return mUniformBufferBlueprints; }
		const std::unordered_map<std::string, SRGBlueprint>& GetSRGBlueprints() const { return mSRGBlueprints; }
		const std::unordered_map<std::string, Texture2DBlueprint>& GetTextureBlueprints() const { return mTexture2DBlueprints; }
	private:

		template<typename T>
		using ResourceBlueprintMap = std::unordered_map<std::string, T>;

		ResourceBlueprintMap<UniformBufferBlueprint> mUniformBufferBlueprints;
		ResourceBlueprintMap<SRGBlueprint> mSRGBlueprints;
		ResourceBlueprintMap<Texture2DBlueprint> mTexture2DBlueprints;

		ResourceHandle mOutputFBHandle;
	};
}