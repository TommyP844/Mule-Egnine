#pragma once

#include "Ref.h"
#include "Graphics/API/GraphicsCore.h"

#include <initializer_list>

namespace Mule
{
	struct ShaderResourceDescription
	{
		uint32_t Binding;
		ShaderResourceType Type;
		ShaderStage Stages;
		uint32_t ArrayCount = 1;
	};

	class ShaderResourceBlueprint
	{
	public:
		static Ref<ShaderResourceBlueprint> Create(const std::initializer_list<ShaderResourceDescription>& resources);

		virtual ~ShaderResourceBlueprint() = default;
	};
}
