#pragma once

#include "Ref.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/Renderer/CommandList.h"
#include "Graphics/Renderer/RenderGraph/ResourceRegistry.h"

namespace Mule::CommandExecutor
{
	void Execute(Ref<CommandBuffer> cmd, const CommandList& commandList, const ResourceRegistry& registry, uint32_t frameIndex);
}
