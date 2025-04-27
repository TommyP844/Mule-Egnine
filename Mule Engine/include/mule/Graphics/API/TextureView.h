#pragma once

#include <imgui.h>

namespace Mule
{
	class TextureView
	{
	public:
		virtual ~TextureView() = default;

		virtual ImTextureID GetImGuiID() const = 0;

	protected:
		TextureView() = default;
	};
}
