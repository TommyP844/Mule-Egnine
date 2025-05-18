#pragma once

#include "Texture.h"

namespace Mule
{
	class Texture2DArray : public Texture
	{
	public:
		static Ref<Texture2DArray> Create(const std::string& name, const Buffer& data, uint32_t width, uint32_t height, uint32_t layers, TextureFormat format, TextureFlags flags);
		virtual ~Texture2DArray() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

	protected:
		Texture2DArray(const std::string& name, TextureFormat format, TextureFlags flags);
	};
}
