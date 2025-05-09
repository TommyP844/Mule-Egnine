#include "Graphics/renderer/RenderGraph/ResourceBuilder.h"

namespace Mule
{	
	ResourceBuilder::ResourceBuilder()
	{
	}

	void ResourceBuilder::InitializeRegistryResources(ResourceRegistry& registry)
	{
		assert(mOutputFBHandle != ResourceHandle::Null() && "Make sure to call ResourceBuilder::SetFramebufferOutputHandle()");
		
		for (auto& [handle, res] : mResourceBlueprints)
		{
			switch (res.first)
			{
			case ResourceType::UniformBuffer:
				registry.AddResource<UniformBuffer>(handle, std::get<UniformBufferBlueprint>(res.second).Size);
				break;

			case ResourceType::ShaderResourceGroup: 
				registry.AddResource<ShaderResourceGroup>(handle, std::get<ShaderResourceGroupBlueprint>(res.second).Descriptions);
				break;

			case ResourceType::Framebuffer: 
				registry.AddResource<Framebuffer>(handle, std::get<FramebufferBlueprint>(res.second).FBDesc);
				break;
			}
		}

		registry.SetOutputFramebufferHandle(mOutputFBHandle);
	}

	void ResourceBuilder::InitializeGlobalResourceRegistry(ResourceRegistry& registry)
	{
		for (auto& [handle, res] : mGlobalBlueprints)
		{
			switch (res.first)
			{
			case ResourceType::UniformBuffer:
				registry.AddResource<UniformBuffer>(handle, std::get<UniformBufferBlueprint>(res.second).Size);
				break;

			case ResourceType::ShaderResourceGroup:
				registry.AddResource<ShaderResourceGroup>(handle, std::get<ShaderResourceGroupBlueprint>(res.second).Descriptions);
				break;

			case ResourceType::Framebuffer:
				registry.AddResource<Framebuffer>(handle, std::get<FramebufferBlueprint>(res.second).FBDesc);
				break;
			}
		}
	}

	ResourceHandle ResourceBuilder::CreateResource(const std::string& name, ResourceType type, uint32_t bufferSize)
	{
		assert(type == ResourceType::UniformBuffer && "Invalid resource type");

		ResourceHandle handle = ResourceHandle::Create(name);
		mResourceBlueprints[handle] = { type, UniformBufferBlueprint {bufferSize} };
		return handle;
	}

	ResourceHandle ResourceBuilder::CreateResource(const std::string& name, const FramebufferDescription& description)
	{
		ResourceHandle handle = ResourceHandle::Create(name);
		mResourceBlueprints[handle] = { ResourceType::Framebuffer, FramebufferBlueprint {description} };
		return handle;
	}

	ResourceHandle ResourceBuilder::CreateResource(const std::string& name, const std::vector<ShaderResourceDescription>& resources)
	{
		ResourceHandle handle = ResourceHandle::Create(name);
		mResourceBlueprints[handle] = { ResourceType::ShaderResourceGroup, ShaderResourceGroupBlueprint {resources} };
		return handle;
	}

	ResourceHandle ResourceBuilder::CreateGlobalResource(const std::string& name, ResourceType type, uint32_t size)
	{
		assert(type == ResourceType::UniformBuffer && "Invalid resource type");

		ResourceHandle handle = ResourceHandle::Create(name);
		mGlobalBlueprints[handle] = { type,  UniformBufferBlueprint { size } };
		return handle;
	}

	ResourceHandle ResourceBuilder::CreateGlobalResource(const std::string& name, const FramebufferDescription& description)
	{
		ResourceHandle handle = ResourceHandle::Create(name);
		mGlobalBlueprints[handle] = { ResourceType::Framebuffer,  FramebufferBlueprint { description } };
		return handle;
	}

	ResourceHandle ResourceBuilder::CreateGlobalResource(const std::string& name, const std::vector<ShaderResourceDescription>& resources)
	{
		ResourceHandle handle = ResourceHandle::Create(name);
		mGlobalBlueprints[handle] = { ResourceType::ShaderResourceGroup,  ShaderResourceGroupBlueprint { resources } };
		return handle;
	}
	
	ResourceHandle ResourceBuilder::GetGlobalResource(const std::string& name)
	{
		return ResourceHandle::Create(name);
	}

	ResourceHandle ResourceBuilder::GetHandle(const std::string& name) const
	{
		return ResourceHandle::Create(name);
	}

	void ResourceBuilder::SetFramebufferOutputHandle(ResourceHandle outputHandle)
	{
		mOutputFBHandle = outputHandle;
	}
}