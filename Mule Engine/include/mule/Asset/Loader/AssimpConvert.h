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

	m[0][0] = mat.a1;
	m[0][1] = mat.a2;
	m[0][2] = mat.a3;
	m[0][3] = mat.a4;

	m[1][0] = mat.b1;
	m[1][1] = mat.b2;
	m[1][2] = mat.b3;
	m[1][3] = mat.b4;

	m[2][0] = mat.c1;
	m[2][1] = mat.c2;
	m[2][2] = mat.c3;
	m[2][3] = mat.c4;

	m[3][0] = mat.d1;
	m[3][1] = mat.d2;
	m[3][2] = mat.d3;
	m[3][3] = mat.d4;

	return m;
}
