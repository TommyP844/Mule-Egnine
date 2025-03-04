#pragma once

#include <glm/glm.hpp>

namespace Mule::GPU
{

	struct GPUCamera
	{
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 Proj;
		alignas(16) glm::vec3 CameraPos;
	};

	struct GPUMaterial
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

	struct GPUDirectionalLight
	{
		alignas(4)  float Intensity = 0.f;
		alignas(16) glm::vec3 Color = glm::vec3(0.f);
		alignas(16) glm::vec3 Direction = glm::vec3(0.f, 1.f, 0.f);
	};

	struct GPUPointLight
	{
		alignas(4)  float Intensity = 0.f;
		alignas(16) glm::vec3 Color = glm::vec3(0.f);
		alignas(16) glm::vec3 Position;
	};

	struct GPULightData
	{
		GPUDirectionalLight DirectionalLight;
		GPUPointLight PointLights[1024];
		alignas(4) uint32_t NumPointLights = 0;
	};
}
