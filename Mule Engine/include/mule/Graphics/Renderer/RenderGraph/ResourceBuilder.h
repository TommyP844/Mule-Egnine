#pragma once

#include "Ref.h"

#include "Graphics/Renderer/RenderGraph/ResourceHandle.h"
#include "Graphics/Renderer/RenderGraph/ResourceRegistry.h"

// Resources
#include "Graphics/API/Framebuffer.h"
#include "Graphics/API/ShaderResourceBlueprint.h"

#include <unordered_map>
#include <variant>

namespace Mule
{
	enum class ResourceType {
		UniformBuffer,
		ShaderResourceGroup,
		Framebuffer
	};

	class ResourceBuilder
	{
	public:
		ResourceBuilder();
		~ResourceBuilder() = default;

		void InitializeRegistryResources(ResourceRegistry& registry);
		void InitializeGlobalResourceRegistry(ResourceRegistry& registry);

		ResourceHandle CreateResource(const std::string& name, ResourceType type, uint32_t bufferSize);
		ResourceHandle CreateResource(const std::string& name, const FramebufferDescription& description);
		ResourceHandle CreateResource(const std::string& name, const std::vector<ShaderResourceDescription>& resources);
				
		ResourceHandle CreateGlobalResource(const std::string& name, ResourceType type, uint32_t size);
		ResourceHandle CreateGlobalResource(const std::string& name, const FramebufferDescription& description);
		ResourceHandle CreateGlobalResource(const std::string& name, const std::vector<ShaderResourceDescription>& resources);
		

		ResourceHandle GetGlobalResource(const std::string& name);
		ResourceHandle GetHandle(const std::string& name) const;
		void SetFramebufferOutputHandle(ResourceHandle outputHandle);
		
	private:

		struct UniformBufferBlueprint { uint32_t Size; };
		struct FramebufferBlueprint { FramebufferDescription FBDesc; };
		struct ShaderResourceGroupBlueprint { std::vector<ShaderResourceDescription> Descriptions; };

		using ResourceVariant = std::variant<
			std::monostate,
			UniformBufferBlueprint,
			FramebufferBlueprint,
			ShaderResourceGroupBlueprint
		>;

		using ResourceBlueprintMap = std::unordered_map<ResourceHandle, std::pair<ResourceType, ResourceVariant>>;

		ResourceBlueprintMap mResourceBlueprints;
		ResourceBlueprintMap mGlobalBlueprints;

		ResourceHandle mOutputFBHandle;
	};
}