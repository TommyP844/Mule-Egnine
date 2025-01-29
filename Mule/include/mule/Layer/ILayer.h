#pragma once

#include <string>

#include "EngineContext.h"

namespace Mule
{
	class ILayer
	{
	public:
		ILayer(Ref<EngineContext> context, const std::string& name = "Layer") : mName(name), mEngineContext(context) {}
		virtual ~ILayer() {}
		ILayer(const ILayer&) = delete;

		virtual void OnEvent(Ref<Event> event) = 0;
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