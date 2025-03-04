
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
    mat4 View;
    mat4 Proj;
	vec3 Pos;
};

layout(binding = 0) uniform CameraBuffer {
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
	_normal = normal;
	_fragPos = (transform * vec4(position, 1.0)).xyz;
	gl_Position = Camera.Proj * Camera.View * transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_CASCADES 10
#define MAX_MATERIALS 800
#define MAX_POINT_LIGHTS 1024

#define PI 3.1415926535897932384626433832795
#define UINT32_MAX 0xFFFFFFFF

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in mat3 TBN;
layout(location = 5) in vec3 normal;

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
	uint OpacityIndex;
	float Transparency;
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

layout(set = 0, binding = 4) uniform samplerCube irradianceMap;
layout(set = 0, binding = 5) uniform samplerCube prefilteredMap;
layout(set = 0, binding = 6) uniform sampler2D brdfLUT;

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(push_constant) uniform PushConstantBlock {
    layout(offset = 64) uint MaterialIndex;
    layout(offset = 68) uint NumCascades;
};


float ndfGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

const float Epsilon = 0.0001;

void main()
{
	Material material = materials[MaterialIndex];

	vec2 scaledUV = uv * material.TextureScale;

	vec3 albedo = material.AlbedoColor.rgb * texture(textures[material.AlbedoIndex], scaledUV).rgb;
    vec3 N;
	if(material.NormalIndex == UINT32_MAX)
	{
		N = normalize(normal);
	}
	else
	{
		vec3 normalTS = texture(textures[material.NormalIndex], scaledUV).xyz * 2.0 - 1.0;
		N = normalize(TBN * normalTS);
	}
    float metalness = texture(textures[material.MetalnessIndex], scaledUV).r * material.MetalnessFactor;
    float roughness = texture(textures[material.RoughnessIndex], scaledUV).r * material.RoughnessFactor;
    float ao = texture(textures[material.AOIndex], scaledUV).r * material.AOFactor;
    
	vec3 Lo = normalize(Camera.Pos - FragPos);
	float cosLo = max(0.0, dot(N, Lo));
	vec3 Lr = normalize(2.0 * cosLo * N - Lo);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metalness);

	vec3 directLighting = vec3(0);

	// Directional Light
	{

		vec3 Li = -Lights.DirectionalLight.Direction;
		vec3 Lradiance = Lights.DirectionalLight.Color * Lights.DirectionalLight.Intensity;

		// Half-vector between Li and Lo.
		vec3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
		vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, roughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		vec3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}


	for(int i=0; i < Lights.NumPointLights; ++i)
	{
		PointLight light = Lights.PointLights[i];

		vec3 Li = normalize(light.Position - FragPos);
		float distance = length(light.Position - FragPos);
		vec3 Lradiance = light.Color * light.Intensity / (distance * distance);

		// Half-vector between Li and Lo.
		vec3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
		vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, roughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		vec3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}

	// Ambient lighting (IBL).
	vec3 ambientLighting;
	{
		// Sample diffuse irradiance at normal direction.
		vec3 irradianceDir = N;
		irradianceDir.y = -irradianceDir.y;
		vec3 irradiance = texture(irradianceMap, irradianceDir).rgb;

		// Calculate Fresnel term for ambient lighting.
		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// use cosLo instead of angle with light's half-vector (cosLh above).
		// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
		vec3 F = fresnelSchlick(F0, cosLo);

		// Get diffuse contribution factor (as with direct lighting).
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		vec3 diffuseIBL = kd * albedo * irradiance;

		// Sample pre-filtered specular reflection environment at correct mipmap level.
		int specularTextureLevels = textureQueryLevels(prefilteredMap);
		vec3 sampleDir = Lr;
		sampleDir.y = -sampleDir.y;
		vec3 specularIrradiance = texture(prefilteredMap, sampleDir/*roughness * specularTextureLevels */).rgb;

		// Split-sum approximation factors for Cook-Torrance specular BRDF.
		vec2 specularBRDF = texture(brdfLUT, vec2(cosLo, roughness)).rg;

		// Total specular IBL contribution.
		vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		// Total ambient lighting contribution.
		ambientLighting = diffuseIBL + specularIBL;
	}

	vec3 finalColor = directLighting + ambientLighting;

	#ifdef TRANSPARENCY

	//float alpha = 1;
	//if(material.OpacityIndex != UINT32_MAX)
	//{
	//	alpha = texture(textures[material.OpacityIndex], scaledUV).r;
	//}

	float alpha = material.Transparency;
	//alpha *= material.AlbedoColor.a;

	FragColor = vec4(vec3(1), alpha);
	#else
	FragColor = vec4(finalColor, 1.0);
	#endif
}