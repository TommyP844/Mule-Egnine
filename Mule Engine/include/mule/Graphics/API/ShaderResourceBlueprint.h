#pragma once

#include "Ref.h"
#include "Graphics/API/GraphicsCore.h"

#include <initializer_list>

namespace Mule
{
	struct ShaderResourceDescription
	{
		ShaderResourceDescription() = default;
		ShaderResourceDescription(uint32_t binding, ShaderResourceType type, ShaderStage stages, uint32_t arrayCount = 1)
			:
			Binding(binding),
			Type(type),
			Stages(stages),
			ArrayCount(arrayCount)
		{ }

		uint32_t Binding;
		ShaderResourceType Type;
		ShaderStage Stages;
		uint32_t ArrayCount = 1;
	};

	class ShaderResourceBlueprint
	{
	public:
		static Ref<ShaderResourceBlueprint> Create(const std::vector<ShaderResourceDescription>& resources);

		virtual ~ShaderResourceBlueprint() = default;
	};
}
