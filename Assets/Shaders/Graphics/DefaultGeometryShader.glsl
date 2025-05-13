#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(location = 0) out vec2 _uv;
layout(location = 1) out vec3 _fragPos;
layout(location = 2) out mat3 _tbn;
layout(location = 5) out vec3 _normal;

struct CameraData
{
    mat4 ViewProj;
    mat4 View;
	vec3 Pos;
	vec3 ViewDir;
};

layout(set = 0, binding = 0) uniform CameraBuffer {
    CameraData Camera;
};

layout(push_constant) uniform PushConstantBlock {
    mat4 transform;
};

void main()
{
	vec3 T = normalize(vec3(transform * vec4(tangent.xyz, 0.0)));
	vec3 N = normalize(vec3(transform * vec4(normal, 0.0)));
	vec3 B = normalize(cross(N, T));
	_tbn = mat3(T, B, N);
	_uv = uv;
	_normal = N;
	_fragPos = (transform * vec4(position, 1.0)).xyz;
	gl_Position = Camera.ViewProj * transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_MATERIALS 800

#define UINT32_MAX 0xFFFFFFFF

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in mat3 TBN;
layout(location = 5) in vec3 normal;

layout(location = 0) out vec4 Albedo;
layout(location = 1) out vec4 Position;
layout(location = 2) out vec4 OutNormal;
layout(location = 3) out vec4 PBRFactors; // Metallic, Roughness, AO, Emissive

struct Material 
{
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
	uint OpacityIndex;
	float Transparency;
};

// Bindless Texture Descriptor Set
layout(set = 1, binding = 0) uniform sampler2D textures[4096];

// Bindless Material Descriptor Set
layout(set = 2, binding = 0) uniform MaterialBuffer {
    Material materials[MAX_MATERIALS];
};

layout(push_constant) uniform PushConstantBlock {
    layout(offset = 64) uint MaterialIndex;
};


void main()
{
	Material material = materials[MaterialIndex];

	vec2 scaledUV = uv * material.TextureScale;

	// Albedo
	Albedo = vec4(material.AlbedoColor.rgb * texture(textures[material.AlbedoIndex], scaledUV).rgb, 1);

	// Normal
	vec3 normalTS = texture(textures[material.NormalIndex], scaledUV).xyz * 2.0 - 1.0;
	OutNormal = vec4(normalize(TBN * normalTS), 1);

	// World Position
	Position = vec4(FragPos, 1);

	// Metallic Factor
    PBRFactors.r = texture(textures[material.MetalnessIndex], scaledUV).r * material.MetalnessFactor;

	// Roughness Factor
    PBRFactors.g = clamp(texture(textures[material.RoughnessIndex], scaledUV).r * material.RoughnessFactor, 0.005, 0.999);

	// Ambient Occlusion
    PBRFactors.b = texture(textures[material.AOIndex], scaledUV).r * material.AOFactor;

	PBRFactors.a = texture(textures[material.EmissiveIndex], scaledUV).r;
}

