#pragma once

#include <glm/glm.hpp>


namespace Mule
{
	struct StaticVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec2 UV;
		glm::vec4 Color;
	};
}
