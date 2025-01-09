#pragma once

#include "Ref.h"
#include "Application/Window.h"
#include "BGFXDebugCallback.h"

namespace Mule
{
	class Renderer
	{
	public:
		static Renderer& Init(Ref<Window> window);
		static void Shutdown();
		static Renderer& Get();

		void NewFrame();
		void RenderFrame();

		void NewImGuiFrame();
		void RenderImGuiFrame();

	private:
		Renderer(Ref<Window> window);
		~Renderer();
		Renderer(const Renderer&) = delete;

		static Renderer* sRenderer;
		
		BGFXDebugCallback callback;
	};
}