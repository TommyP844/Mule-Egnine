#pragma once


#include "Ref.h"

#include "Services/ServiceManager.h"

namespace Mule
{
	class IAssetGenerator
	{
	public:
		IAssetGenerator(Ref<ServiceManager> serviceManager)
			:
			mServiceManager(serviceManager)
		{ }

		virtual ~IAssetGenerator() = default;

	protected:
		Ref<ServiceManager> mServiceManager;
	};
}