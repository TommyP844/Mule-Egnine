#pragma once

#include "Ref.h"
#include "Texture.h"
#include "Buffer.h"
#include "Graphics/API/GraphicsCore.h"
#include "Graphics/API/GraphicsContext.h"
#include "Graphics/API/GraphicsCore.h"
#include "Asset/Asset.h"

#include <filesystem>
#include <string>

namespace Mule
{
	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& name, const Buffer& data, uint32_t width, uint32_t height, TextureFormat format, TextureFlags flags);
		virtual ~Texture2D() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

	protected:
		Texture2D(const std::string& name, TextureFormat format, TextureFlags flags);
	};
}
