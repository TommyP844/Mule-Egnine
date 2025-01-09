#pragma once

#include <string>

#include "Application/ApplicationData.h"

namespace Mule
{
	class ILayer
	{
	public:
		ILayer(WeakRef<ApplicationData> appData, const std::string& name = "Layer") : mName(name), mApplicationData(appData) {}
		~ILayer() {}
		ILayer(const ILayer&) = delete;

		virtual void OnAttach() = 0;
		virtual void OnUpdate(float dt) = 0;
		virtual void OnUIRender() = 0;
		// TODO add OnEvent
		virtual void OnDetach() = 0;

		const std::string& GetName() const { return mName; }

	protected:
		WeakRef<ApplicationData> mApplicationData;
	private:
		std::string mName;
	};
}