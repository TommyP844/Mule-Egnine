#pragma once

#include <string>

#include "EngineContext.h"

namespace Mule
{
	class ILayer
	{
	public:
		ILayer(Ref<EngineContext> context, const std::string& name = "Layer") : mName(name), mEngineContext(context) {}
		~ILayer() {}
		ILayer(const ILayer&) = delete;

		virtual void OnAttach() = 0;
		virtual void OnUpdate(float dt) = 0;
		virtual void OnUIRender() = 0;
		// TODO add OnEvent
		virtual void OnDetach() = 0;

		const std::string& GetName() const { return mName; }

	protected:
		WeakRef<EngineContext> mEngineContext;
	private:
		std::string mName;
	};
}