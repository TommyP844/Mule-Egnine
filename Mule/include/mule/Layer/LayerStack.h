#pragma once

#include "ILayer.h"

namespace Mule
{
	class LayerStack
	{
	public:

		template<typename T>
		void PushLayer(WeakRef<ApplicationData> appData)
		{
			Ref<T> layer = Ref<T>::Make(appData);
			layer->OnAttach();
			SPDLOG_INFO("Layer added to application stack: {}", layer->GetName());
			mLayers.push_back(layer);
		}

		void PopLayer()
		{
			Ref<ILayer> layer = mLayers.back();
			SPDLOG_INFO("Layer removed to application stack: {}", layer->GetName());
			mLayers.pop_back();
			layer->OnDetach();
		}

		bool empty() const { return mLayers.empty(); }

		std::vector<Ref<ILayer>>::iterator begin() { return mLayers.begin(); }
		std::vector<Ref<ILayer>>::iterator end() { return mLayers.end(); }

	private:
		std::vector<Ref<ILayer>> mLayers;
	};
}
