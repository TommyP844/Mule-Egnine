
#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(location = 0) out vec2 _uv;
layout(location = 1) out vec3 _normal;
layout(location = 2) out vec3 _fragPos;

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

const mat4 bias = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );

void main()
{
	_uv = uv;
	_fragPos = (transform * vec4(position, 1.0)).xyz;
	gl_Position = Camera.Proj * Camera.View * transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_CASCADES 10
#define MAX_MATERIALS 1000
#define MAX_POINT_LIGHTS 1024

#define PI 3.1415926535897932384626433832795

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 FragPos;

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


// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 prefilteredReflection(vec3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilteredMap, R, lodf).rgb;
	vec3 b = textureLod(prefilteredMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

vec3 specularContribution(vec3 lightColor, vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, F0);		
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
		color += (kD * lightColor / PI + spec) * dotNL;
	}

	return color;
}

vec3 getNormalFromMap(vec3 normal, uint textureIndex, vec2 uv, vec3 fragPos)
{
    vec3 tangentNormal = texture(textures[textureIndex], uv).rgb * 2.0 - 1.0; // Convert to [-1,1]

    vec3 Q1  = dFdx(fragPos);
    vec3 Q2  = dFdy(fragPos);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N  = normalize(normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
	Material material = materials[MaterialIndex];

	vec2 scaledUV = uv * material.TextureScale;

	vec3 albedo = material.AlbedoColor.xyz * texture(textures[material.AlbedoIndex], scaledUV).rgb;
    vec3 N = getNormalFromMap(normal, material.NormalIndex, scaledUV, FragPos);
    float metallic = texture(textures[material.MetalnessIndex], scaledUV).r * material.MetalnessFactor;
    float roughness = texture(textures[material.RoughnessIndex], scaledUV).r * material.RoughnessFactor;
    float ao = texture(textures[material.AOIndex], scaledUV).r * material.AOFactor;
    
	vec3 V = normalize(Camera.Pos - FragPos);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < Lights.NumPointLights; i++) {
		vec3 L = normalize(Lights.PointLights[i].Position - FragPos);
		float D = length(Lights.PointLights[i].Position - FragPos);
		vec3 luminance = specularContribution(Lights.PointLights[i].Color, L, V, N, F0, metallic, roughness);
		Lo += (luminance * Lights.PointLights[i].Intensity) / (D*D);
	}
	vec3 luminance = specularContribution(Lights.DirectionalLight.Color, -Lights.DirectionalLight.Direction, V, N, F0, metallic, roughness);
	Lo += luminance * Lights.DirectionalLight.Intensity;
	
	vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	//vec3 reflection = prefilteredReflection(R, roughness).rgb;	
	vec3 irradiance = texture(irradianceMap, N).rgb;

	// Diffuse based on irradiance
	vec3 diffuse = irradiance * albedo;	

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	// Specular reflectance
	vec3 specular = vec3(0);//reflection * (F * brdf.x + brdf.y);

	// Ambient part
	vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;	  
	vec3 ambient = (kD * diffuse + specular);
	
	vec3 color = ambient + Lo;

	// Tone mapping
	color = Uncharted2Tonemap(color * 0.7);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	// Gamma correction
	color = pow(color, vec3(1.0f / 2.2));

	FragColor = vec4(color, 1.0);
}