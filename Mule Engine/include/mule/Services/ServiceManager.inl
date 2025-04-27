#pragma once

#include <spdlog/spdlog.h>
#include "ServiceManager.h"

namespace Mule
{
	template<class T, typename ...Args>
	inline WeakRef<T> ServiceManager::Register(Args && ...args)
	{
		const std::type_index typeIndex = std::type_index(typeid(T));

		auto iter = mServices.find(typeIndex);
		if (iter != mServices.end())
		{
			SPDLOG_INFO("Service already registered: {}", typeid(T).name());
			return nullptr;
		}

		SPDLOG_INFO("Registering service: {}", typeid(T).name());

		Ref<Service<T>> service = MakeRef<Service<T>>(std::forward<Args>(args)...);

		mServices[typeIndex] = service;
		return service->Get();
	}

	template<class T>
	inline WeakRef<T> ServiceManager::Get() const
	{
		const std::type_index typeIndex = std::type_index(typeid(T));

		auto iter = mServices.find(typeIndex);
		if (iter == mServices.end())
		{
			SPDLOG_INFO("Service not found: {}", typeid(T).name());
			return nullptr;
		}

		WeakRef<Service<T>> service = iter->second;

		return service->Get();
	}
	template<class Service>
	inline void Mule::ServiceManager::Unload()
	{
		const std::type_index typeIndex = std::type_index(typeid(Service));

		auto iter = mServices.find(typeIndex);
		if (iter == mServices.end())
		{
			SPDLOG_INFO("Service not found: {}", typeid(Service).name());
			return;
		}

		mServices.erase(typeIndex);
	}
}