#pragma once

#include <glm/glm.hpp>

#include <assimp/vector2.h>
#include <assimp/matrix4x4.h>
#include <assimp/color4.h>


static glm::vec2 toGlm(const aiVector2D& vec)
{
	return glm::vec2(vec.x, vec.y);
}

static glm::vec3 toGlm(const aiVector3D& vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

static glm::vec4 toGlm(const aiColor4D& vec)
{
	return glm::vec4(vec.r, vec.g, vec.b, vec.a);
}

static glm::mat4 toGlm(const aiMatrix4x4& mat)
{
	glm::mat4 m;

	for (uint32_t i = 0; i < 4; i++)
	{
		for (uint32_t j = 0; j < 4; j++)
		{
			m[j][i] = mat[i][j];
		}
	}

	return m;
}
