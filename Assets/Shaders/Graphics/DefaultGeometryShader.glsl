
#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(location = 0) out vec2 _uv;
layout(location = 1) out mat3 _tbn;
layout(location = 5) out vec3 _fragPos;

struct CameraData
{
    mat4 View;
    mat4 Proj;
	vec3 Pos;
    float FarPlane;
};

layout(binding = 0) uniform CameraBuffer {
    CameraData Camera;
};

layout(push_constant) uniform PushConstantBlock {
    mat4 transform;
};

void main()
{
	vec3 T = normalize(vec3(transform * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(transform * vec4(normal, 0.0)));
    vec3 B = cross(N, T);
    _tbn = mat3(T, B, N);
	_uv = uv;
	_fragPos = (transform * vec4(position, 1)).xyz;
	gl_Position = Camera.Proj * Camera.View * vec4(_fragPos, 1);
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_CASCADES 10
#define MAX_MATERIALS 1000
#define MAX_POINT_LIGHTS 1024

layout(location = 0) in vec2 uv;
layout(location = 1) in mat3 TBN;
layout(location = 5) in vec3 FragPos;

layout(location = 0) out vec4 FragColor;

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
};

struct DirectionalLight
{
	float Intensity;
	vec3 Color;
	vec3 Direction;
};

struct PointLight
{
	float Intensity;
	vec3 Color;
	vec3 Position;
};

struct LightData
{
	DirectionalLight DirectionalLight;
	PointLight PointLights[MAX_POINT_LIGHTS];
	uint NumPointLights;
};

struct CameraData
{
    mat4 View;
    mat4 Proj;
	vec3 Pos;
    float FarPlane;
};

layout(binding = 0) uniform CameraBuffer {
    CameraData Camera;
};

layout(binding = 1) uniform MaterialBuffer {
    Material materials[MAX_MATERIALS];
};

layout(binding = 2) uniform LightBuffer {
	LightData Lights;
};

layout(binding = 3) uniform ShadowCameraBuffer {
	mat4 ShadowCameras[MAX_CASCADES];
    float CascadeDistances[MAX_CASCADES];
};

layout(set = 0, binding = 4) uniform samplerCube irradianceMap;
layout(set = 0, binding = 5) uniform samplerCube prefilteredMap;
layout(set = 0, binding = 6) uniform sampler2D brdfLUT;
layout(set = 0, binding = 7) uniform sampler2D shadowMaps[MAX_CASCADES];

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(push_constant) uniform PushConstantBlock {
    layout(offset = 64) uint MaterialIndex;
    layout(offset = 68) uint NumCascades;
};

// Fresnel-Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX Normal Distribution Function (NDF)
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265 * denom * denom;

    return num / denom;
}

// Schlick-GGX Geometry Function
float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Smith Geometry Function for Both View & Light
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometrySchlickGGX(NdotV, roughness);
    float ggx2 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 computePBR(vec3 albedo, vec3 normal, vec3 V, vec3 L, vec3 lightColor, float intensity, float metallic, float roughness) {
    vec3 H = normalize(V + L);
    float NDF = distributionGGX(normal, H, roughness);
    float G = geometrySmith(normal, V, L, roughness);

    // Base Reflectivity (Fresnel-Schlick Approximation)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Cook-Torrance BRDF
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // Diffuse Component (Lambertian Reflection)
    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kd * albedo / 3.14159265;

    // Final Light Contribution
    float NdotL = max(dot(normal, L), 0.0);
    return (diffuse + specular) * lightColor * intensity * NdotL;
}

vec3 computeDirectionalLight(vec3 albedo, vec3 normal, vec3 V, DirectionalLight light, float metallic, float roughness) {
    vec3 L = normalize(-light.Direction);
    return computePBR(albedo, normal, V, L, light.Color, light.Intensity, metallic, roughness);
}

vec3 computePointLight(vec3 albedo, vec3 normal, vec3 V, PointLight light, float metallic, float roughness) {
    vec3 L = normalize(light.Position - FragPos);
	float distance = length(FragPos - light.Position);
	float attenuation = 1.0 / (distance * distance);
    return computePBR(albedo, normal, V, L, light.Color, light.Intensity * attenuation, metallic, roughness);
}

vec3 diffuseIBL(vec3 normal, vec3 albedo, float ao) {
    vec3 irradiance = texture(irradianceMap, normal).rgb;
    return irradiance * albedo * ao;
}

vec3 specularIBL(vec3 R, float roughness, vec3 F0, vec3 N, vec3 V) {
    float mipLevel = roughness * 1; // MAX_MIP_LEVEL Adjusted for roughness
    vec3 prefilteredColor = textureLod(prefilteredMap, R, mipLevel).rgb;

    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    return prefilteredColor * (F0 * brdf.x + brdf.y);
}


void main()
{
	Material material = materials[MaterialIndex];

	vec2 scaledUV = uv * material.TextureScale;

	vec3 albedo = material.AlbedoColor.xyz * texture(textures[material.AlbedoIndex], scaledUV).rgb;
    vec3 normal = texture(textures[material.NormalIndex], scaledUV).rgb * 2.0 - 1.0; // Convert to [-1,1]
    normal = normalize(TBN * normal);
    float metallic = texture(textures[material.MetalnessIndex], scaledUV).r * material.MetalnessFactor;
    float roughness = texture(textures[material.RoughnessIndex], scaledUV).r * material.RoughnessFactor;
    float ao = texture(textures[material.AOIndex], scaledUV).r * material.AOFactor;
    
	vec3 V = normalize(Camera.Pos - FragPos);
	vec3 lighting = vec3(0.0);

	lighting += computeDirectionalLight(albedo, normal, V, Lights.DirectionalLight, metallic, roughness);
	for(int i = 0; i < Lights.NumPointLights; i++)
	{
		lighting += computePointLight(albedo, normal, V, Lights.PointLights[i], metallic, roughness);
	}

	lighting *= ao;
	vec3 reflectionDir = reflect(V, normal);
	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 ambientLighting = diffuseIBL(normal, albedo, ao);// + specularIBL(reflectionDir, roughness, F0, normal, V);

    vec3 hdrColor = ambientLighting + lighting;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * 0.9);
    mapped = pow(mapped, vec3(1.0 / 2.2));

	FragColor = vec4(mapped, 1.0);
}