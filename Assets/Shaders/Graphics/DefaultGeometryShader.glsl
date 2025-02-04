
#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(location = 0) out vec2 _uv;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstantBlock {
    mat4 transform;
} pc;

void main()
{
	_uv = uv;
	gl_Position = ubo.proj * ubo.view * pc.transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 1) uniform sampler2D textures[];

struct Material {
	vec4 AlbedoColor;
	vec2 TextureScale;
	float MetalnessFactor;
	float RoughnessFactor;
	float AOFactor;
	uint AlbedoIndex;
	uint NormalIndex;
	uint MetalnessIndex;
	uint RoughnessIndex;
	uint AOIndex;
	uint EmissiveIndex;
};

layout(binding = 2) uniform UniformBufferObject {
    Material materials[100];
} materials;

layout(push_constant) uniform PushConstantBlock {
    layout(offset = 64) uint materialIndex;
} pc;

void main()
{
	Material material = materials.materials[pc.materialIndex];

	fragColor = vec4(material.AlbedoColor.xyz, 1) * vec4(texture(textures[material.AlbedoIndex], uv).xyz, 1);
}