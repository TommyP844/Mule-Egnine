#pragma once

#include "Ref.h"
#include "Application/Window.h"
#include "BGFXDebugCallback.h"

namespace Mule
{
	class MasterRenderer
	{
	public:
		static MasterRenderer& Init(Ref<Window> window);
		static void Shutdown();
		static MasterRenderer& Get();

		void NewFrame();
		void RenderFrame();

		void NewImGuiFrame();
		void RenderImGuiFrame();

	private:
		MasterRenderer(Ref<Window> window);
		~MasterRenderer();
		MasterRenderer(const MasterRenderer&) = delete;

		static MasterRenderer* sRenderer;
		
		BGFXDebugCallback callback;
	};
}