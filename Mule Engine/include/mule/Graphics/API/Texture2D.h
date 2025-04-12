#pragma once

#include "Ref.h"
#include "Graphics/API/GraphicsCore.h"
#include "Graphics/API/GraphicsContext.h"
#include "Graphics/API/GraphicsCore.h"
#include "Asset/Asset.h"

#include <filesystem>
#include <string>

namespace Mule
{

	class Texture2D
	{
	public:
		static Ref<Texture2D> Create(void* data, uint32_t width, uint32_t height, TextureFormat format, TextureFlags flags);

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual TextureFormat GetFormat() const = 0;
		virtual TextureFlags GetFlags() const = 0;

	protected:
		Texture2D() = default;
	};
}
