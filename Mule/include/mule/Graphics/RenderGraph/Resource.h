#pragma once

#include "Ref.h"

namespace Mule::RenderGraph
{
	enum class ResourceType
	{
		StorageImage,
		Texture,
		UniformBuffer,
		StorageBuffer,
		DescriptorSet,
		FrameBuffer,
		RenderPass
	};

	class IResource
	{
	public:
		IResource(ResourceType type)
			:
			mType(type)
		{ }

		virtual ~IResource(){}

		ResourceType GetType() const { return mType; }

	private:
		ResourceType mType;
	};

	template<typename T>
	class Resource : public IResource
	{
	public:
		Resource(Ref<T> resource, ResourceType type)
			:
			IResource(type),
			mResource(resource)
		{ }

		virtual ~IResource() {}

		Ref<T> Get() const {return mResource}

	private:
		Ref<T> mResource;
	};
}