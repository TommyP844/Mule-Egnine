#pragma once

#include "WeakRef.h"

namespace Mule
{
	class IService
	{
	public:
		virtual ~IService() {}
	};

	template<typename T>
	class Service : public IService
	{
	public:
		template<typename ...Args>
		Service(Args&&... args)
		{
			mService = MakeRef<T>(std::forward<Args>(args)...);
		}

		WeakRef<T> Get() const { return mService; }

	private:
		Ref<T> mService;
	};
}