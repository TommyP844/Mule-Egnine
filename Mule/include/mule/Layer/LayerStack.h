#pragma once

#include "ILayer.h"

namespace Mule
{
	class LayerStack
	{
	public:

		template<typename T>
		void PushLayer(Ref<EngineContext> context)
		{
			Ref<T> layer = MakeRef<T>(context);
			layer->OnAttach();
			SPDLOG_INFO("Layer added to application stack: {}", layer->GetName());
			mLayers.push_back(layer);
		}

		void PopLayer()
		{
			Ref<ILayer> layer = mLayers.back();
			layer->OnDetach();
			mLayers[mLayers.size() - 1] = nullptr; // Force destruction of layer
			mLayers.pop_back();
			SPDLOG_INFO("Layer removed to application stack: {}", layer->GetName());
		}

		bool empty() const { return mLayers.empty(); }

		std::vector<Ref<ILayer>>::iterator begin() { return mLayers.begin(); }
		std::vector<Ref<ILayer>>::iterator end() { return mLayers.end(); }

	private:
		std::vector<Ref<ILayer>> mLayers;
	};
}
