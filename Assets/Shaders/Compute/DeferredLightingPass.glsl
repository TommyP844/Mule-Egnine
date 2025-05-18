#COMPUTE
#version 450 core

#define MAX_POINT_LIGHTS 1000
#define MAX_SPOT_LIGHTS 800
#define MAX_CASCADES 10
#define PI 3.14159265359

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

struct SpotLight
{
	float Intensity;
	vec3 Color;
	vec3 Position;
	vec3 Direction;
	float HalfAngle;
	float FallOff;
};

struct CameraData
{
    mat4 ViewProj;
    mat4 View;
	mat4 Proj;
	vec3 Pos;
	vec3 ViewDir;
};

layout(set = 0, binding = 0, rgba32f) uniform writeonly image2D ColorBuffer;
layout(set = 0, binding = 1) uniform sampler2D gAlbedo;
layout(set = 0, binding = 2) uniform sampler2D gPosition;
layout(set = 0, binding = 3) uniform sampler2D gNormal;
layout(set = 0, binding = 4) uniform sampler2D gPBR;

layout(set = 1, binding = 0) uniform DirectionalLightBuffer {
	DirectionalLight DLight;
};

layout(set = 1, binding = 1) uniform PointLightBuffer {
	uint PointLightCount;
	PointLight PointLights[MAX_POINT_LIGHTS];
};

layout(set = 1, binding = 2) uniform SpotLightBuffer {
	uint SpotLightCount;
	SpotLight SpotLights[MAX_SPOT_LIGHTS];
};

layout(set = 2, binding = 0) uniform CameraBuffer {
	CameraData Camera;
};


layout(set = 3, binding = 0) uniform sampler2DArray shadowMap;
layout(set = 3, binding = 1) uniform Cascades {
    mat4 cascadeViewProj[MAX_CASCADES];
    vec4 cascadeSplits[MAX_CASCADES];
    uint cascadeCount;
};

layout(set = 4, binding = 0) uniform samplerCube diffuseIBL;
layout(set = 4, binding = 1) uniform samplerCube prefilterIBL;
layout(set = 4, binding = 2) uniform sampler2D BRDFLut;


vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}

const mat4 biasMatrix = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, -0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);

uint GetCascadeIndex(vec3 worldPos)
{
	float depthValue = (Camera.View * vec4(worldPos, 1.0)).z;

    uint cascadeIndex = 0;
	for(uint i = 0; i < cascadeCount - 1; ++i) {
		if(depthValue < cascadeSplits[i].x) {	
			cascadeIndex = i + 1;
		}
	}

	return cascadeIndex;
}


float GetShadow(vec3 worldPos, vec3 normal, vec3 lightDir)
{
    uint layer = GetCascadeIndex(worldPos);

    // Transform to light space
    vec4 shadowCoord = (biasMatrix * cascadeViewProj[layer]) * vec4(worldPos, 1.0);
    shadowCoord /= shadowCoord.w;

    float shadow = 0.0;
    int count = 0;

	float angle = max(dot(normal, lightDir), 0.0);
    float bias = max(0.0005 * (1.0 - angle), 0.00005);

	ivec2 texSize = textureSize(shadowMap, 0).xy;
    vec2 texelSize = 1.0 / vec2(texSize);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize;
            vec2 sampleUV = shadowCoord.xy + offset;


            float closestDepth = texture(shadowMap, vec3(sampleUV, layer)).r;
            if (closestDepth < shadowCoord.z - bias)
            {
                shadow += 1.0;
            }
            count++;
        }
    }

    return 1.0 - (shadow / float(count));
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
	// ivec2 size = textureSize(ColorBuffer, 0).xy;
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);

	//if(uv.x > size.x || uv.y > size.y)
	//	return;

	vec3 albedo = texelFetch(gAlbedo, uv, 0).rgb;
	vec3 worldPos = texelFetch(gPosition, uv, 0).rgb;
	vec3 normal = normalize(texelFetch(gNormal, uv, 0).rgb);
	vec3 pbr = texelFetch(gPBR, uv, 0).rgb;

	float metallic = pbr.r;
	float roughness = pbr.g;
	float ao = pbr.b;

	vec3 N = normalize(normal);
	vec3 V = normalize(Camera.Pos - worldPos);
	vec3 R = reflect(-V, N);
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	vec3 Lo = vec3(0.0); // Accumulated light

	// Directional Light
	{
		vec3 L = normalize(-DLight.Direction);
		vec3 H = normalize(V + L);

		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 numerator = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		vec3 specular = numerator / denom;

		float NdotL = max(dot(N, L), 0.0);
		vec3 kD = vec3(1.0) - F;
		kD *= 1.0 - metallic;

		vec3 radiance = DLight.Intensity * DLight.Color;

		Lo += (kD * albedo / PI + specular) * radiance * NdotL * GetShadow(worldPos, N, L);
	}

	// Point Lights
	for (uint i = 0; i < PointLightCount; ++i) {
		PointLight light = PointLights[i];
		vec3 L = normalize(light.Position - worldPos);
		vec3 H = normalize(V + L);
		float distance = length(light.Position - worldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = light.Intensity * light.Color * attenuation;

		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 numerator = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		vec3 specular = numerator / denom;

		float NdotL = max(dot(N, L), 0.0);
		vec3 kD = vec3(1.0) - F;
		kD *= 1.0 - metallic;

		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	// Spot Lights
	for (uint i = 0; i < SpotLightCount; ++i) {
		SpotLight light = SpotLights[i];
		vec3 L = normalize(light.Position - worldPos);
		vec3 H = normalize(V + L);
		float distance = length(light.Position - worldPos);
		float attenuation = 1.0 / (distance * distance);

		float spotAngle = dot(normalize(-L), normalize(light.Direction));
		float cutoff = cos(light.HalfAngle);
		float spotEffect = smoothstep(cutoff, cutoff + light.FallOff, spotAngle);

		vec3 radiance = light.Intensity * light.Color * attenuation * spotEffect;

		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 numerator = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		vec3 specular = numerator / denom;

		float NdotL = max(dot(N, L), 0.0);
		vec3 kD = vec3(1.0) - F;
		kD *= 1.0 - metallic;

		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	// IBL: Image-Based Lighting
	{
		vec3 kS = FresnelSchlick(max(dot(N, V), 0.0), F0);
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;
		
		// Diffuse IBL
		vec3 irradiance = texture(diffuseIBL, N).rgb;
		vec3 diffuseIBLColor = irradiance * albedo;
		
		// Specular IBL
		int mipLevels = textureQueryLevels(prefilterIBL);
		float lod = roughness * float(mipLevels - 1); // -1 because mip levels are zero-based
		vec3 prefilteredColor = textureLod(prefilterIBL, R, lod).rgb;
		vec2 brdf = texture(BRDFLut, vec2(max(dot(N, V), 0.0), roughness)).rg;
		vec3 specularIBLColor = prefilteredColor * (kS * brdf.x + brdf.y);
		
		// Combine
		vec3 ambient = (kD * diffuseIBLColor + specularIBLColor) * ao;
		Lo += ambient;
	}

	// Ambient
	vec3 ambient = vec3(0.03) * albedo * ao;

	vec3 color = ambient + Lo;

	// Apply optional tonemapping and gamma correction (if needed)
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0 / 2.2));


	imageStore(ColorBuffer, uv, vec4(color, 1.0));
}