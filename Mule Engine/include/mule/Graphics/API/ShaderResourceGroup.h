#pragma once

#include "ShaderResourceBlueprint.h"

#include "Graphics/API/Texture.h"
#include "Graphics/API/UniformBuffer.h"
#include "Graphics/API/TextureView.h"

namespace Mule
{
	class ShaderResourceGroup
	{
	public:
		static Ref<ShaderResourceGroup> Create(const std::vector<Ref<ShaderResourceBlueprint>>& blueprints);

		// Binds a texture to a resource binding
		virtual void Update(uint32_t binding, WeakRef<Texture> texture, uint32_t arrayIndex = 0) = 0;

		// Binds a Textures View (i.e. a specific mip level or array layer) to a resource binding
		virtual void Update(uint32_t binding, WeakRef<TextureView> texture, uint32_t arrayIndex = 0) = 0;

		// Binds a Uniform buffer to a resource binding
		virtual void Update(uint32_t binding, WeakRef<UniformBuffer> buffer, uint32_t arrayIndex = 0) = 0;

		virtual ~ShaderResourceGroup() = default;
	};
}
