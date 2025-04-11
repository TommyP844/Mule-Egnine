#pragma once

#include "WeakRef.h"
#include "Ref.h"
#include "IService.h"

#include <unordered_map>
#include <typeindex>

namespace Mule
{
	class ServiceManager
	{
	public:
		ServiceManager();
		~ServiceManager();

		template<class Service, typename ... Args>
		WeakRef<Service> Register(Args&&... args);

		template<class Service>
		WeakRef<Service> Get() const;

	private:
		std::unordered_map<std::type_index, Ref<IService>> mServices;
	};
}

#include "ServiceManager.inl"