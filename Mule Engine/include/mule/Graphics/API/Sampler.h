#pragma once

#include "Ref.h"

#include "Graphics/API/GraphicsCore.h"

namespace Mule
{
	struct SamplerDescription
	{
		SamplerDescription() = default;

		SamplerFilterMode MinFilterMode = SamplerFilterMode::Linear;
		SamplerFilterMode MagFilterMode = SamplerFilterMode::Linear;
		SamplerAddressMode AddressModeU = SamplerAddressMode::Repeat;
		SamplerAddressMode AddressModeV = SamplerAddressMode::Repeat;
		SamplerAddressMode AddressModeW = SamplerAddressMode::Repeat;
		float MinLod = 0.f;
		float MaxLod = 1.f;
		MipMapMode MipMapMode = MipMapMode::Linear;
		bool AnisotropyEnable = false;
		float MaxAnisotropy = 1.f;
		SamplerBorderColor BorderColor = SamplerBorderColor::None;
	};

	class Sampler
	{
	public:
		~Sampler() = default;

		static Ref<Sampler> Create(const SamplerDescription& description);

	protected:
		Sampler() = default;
	};
}