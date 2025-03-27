#pragma once

#include "Ref.h"

namespace Mule::RenderGraph
{
	class IResource
	{
	public:
		IResource() { }

		virtual ~IResource(){}
	};

	template<typename T>
	class Resource : public IResource
	{
	public:
		Resource(Ref<T> resource)
			:
			IResource(),
			mResource(resource)
		{ }

		virtual ~Resource() {}

		Ref<T> Get() const { return mResource; }

	private:
		Ref<T> mResource;
	};
}