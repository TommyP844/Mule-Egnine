#pragma once

#include "Asset/Asset.h"
#include "Texture.h"

#include "Ref.h"
#include "Buffer.h"

namespace Mule
{
	class TextureCube : public Texture
	{
	public:
		static Ref<TextureCube> Create(const std::string& name, const Buffer& buffer, uint32_t width, TextureFormat format, TextureFlags flags);

		virtual ~TextureCube() = default;

	protected:
		TextureCube(const std::string& name, TextureFormat format, TextureFlags flags);
	};
}
