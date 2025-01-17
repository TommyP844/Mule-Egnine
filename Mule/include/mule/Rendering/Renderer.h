#pragma once

#include "GraphicsContext.h"


namespace Mule
{
	class Renderer
	{
	public:
		Renderer(Ref<GraphicsContext> context);
		~Renderer();

		void Render();

	private:

	};
}