#pragma once

#include <string>

namespace Mule::RenderGraph
{
	struct RenderPassStats
	{
		std::string Name;
		float CPUExecutionTime = 0.f;
		float GPUExecutionTime = 0.f;
	};
}
