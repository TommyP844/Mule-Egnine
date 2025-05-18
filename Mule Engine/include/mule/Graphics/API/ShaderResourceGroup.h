#pragma once

#include "ShaderResourceBlueprint.h"

#include "Graphics/API/Texture.h"
#include "Graphics/API/UniformBuffer.h"
#include "Graphics/API/TextureView.h"
#include "Graphics/API/Sampler.h"

namespace Mule
{
	class ShaderResourceGroup
	{
	public:
		static Ref<ShaderResourceGroup> Create(Ref<ShaderResourceBlueprint> blueprint);

		static Ref<ShaderResourceGroup> Create(const std::vector<ShaderResourceDescription>& blueprintDescriptions);

		// Binds a texture to a resource binding
		virtual void Update(uint32_t binding, DescriptorType type, ImageLayout layout, WeakRef<Texture> texture, uint32_t arrayIndex = 0, Ref<Sampler> sampler = nullptr) = 0;

		// Binds a Textures View (i.e. a specific mip level or array layer) to a resource binding
		virtual void Update(uint32_t binding, DescriptorType type, ImageLayout layout, WeakRef<TextureView> texture, uint32_t arrayIndex = 0, Ref<Sampler> sampler = nullptr) = 0;

		// Binds a Uniform buffer to a resource binding
		virtual void Update(uint32_t binding, WeakRef<UniformBuffer> buffer, uint32_t arrayIndex = 0) = 0;

		virtual ~ShaderResourceGroup() = default;

		Ref<ShaderResourceBlueprint> GetBlueprint() const { return mBlueprint; }

	protected:
		Ref<ShaderResourceBlueprint> mBlueprint;
	};
}
