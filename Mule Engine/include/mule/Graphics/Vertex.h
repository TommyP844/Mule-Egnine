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

	struct UIVertex
	{
		glm::vec2 Position;
		glm::vec2 UV;
		glm::vec4 Color;
		uint32_t TextureIndex;
		uint32_t IsFont; // 0 for false, 1 for true
	};
}
