#pragma once

#include <glm/glm.hpp>

namespace Mule::GPU
{
	struct Camera
	{
		alignas(16) glm::mat4 ViewProjection;
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 Proj;
		alignas(16) glm::vec3 Position;
		alignas(16) glm::vec3 ViewDirection;
	};

	struct Material
	{
		alignas(16) glm::vec4 AlbedoColor;
		alignas(8) glm::vec2 TextureScale;
		alignas(4) float MetalnessFactor;
		alignas(4) float RoughnessFactor;
		alignas(4) float AOFactor;
		alignas(4) uint32_t AlbedoIndex;
		alignas(4) uint32_t NormalIndex;
		alignas(4) uint32_t MetalnessIndex;
		alignas(4) uint32_t RoughnessIndex;
		alignas(4) uint32_t AOIndex;
		alignas(4) uint32_t EmissiveIndex;
		alignas(4) uint32_t OpacityIndex;
		alignas(4) float Transparency;
	};

	struct DirectionalLight
	{
		alignas(4)  float Intensity = 0.f;
		alignas(16) glm::vec3 Color = glm::vec3(0.f);
		alignas(16) glm::vec3 Direction = glm::vec3(0.f, 1.f, 0.f);
	};

	struct PointLight
	{
		alignas(4)  float Intensity = 0.f;
		alignas(16) glm::vec3 Color = glm::vec3(0.f);
		alignas(16) glm::vec3 Position;
	};

	struct PointLightArray
	{
		alignas(4) uint32_t Count;
		PointLight Lights[1000];
	};

	struct SpotLight
	{
		alignas(4) float Intensity;
		alignas(16) glm::vec3 Color = glm::vec3(0.f);
		alignas(16) glm::vec3 Position;
		alignas(16) glm::vec3 Direction;
		alignas(4) float HalfAngle;
		alignas(4) float FallOff;
	};

	struct SpotLightArray
	{
		alignas(4) uint32_t Count;
		SpotLight Lights[800];
	};

	struct CascadedShadowLightMatrices
	{
		alignas(16) glm::mat4 LightSpaceMatrices[10];
		alignas(16) glm::vec4 CascadeSplits[10];
		alignas(4) uint32_t CascadeCount;
	};
}
