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
	_normal = N;
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
layout(location = 1) out uvec2 EntityId;

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

layout(binding = 3) uniform samplerCube irradianceMap;
layout(binding = 4) uniform samplerCube prefilterMap;
layout(binding = 5) uniform sampler2D brdfLUT;

layout(set = 1, binding = 0) uniform sampler2D textures[4096];

layout(push_constant) uniform PushConstantBlock {
    layout(offset = 64) uint MaterialIndex;
    uvec2 Id;
};


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   


void main()
{
	Material material = materials[MaterialIndex];

	vec2 scaledUV = uv * material.TextureScale;

	vec3 albedo = pow(material.AlbedoColor.rgb * texture(textures[material.AlbedoIndex], scaledUV).rgb, vec3(2.2));
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
    float metallic = texture(textures[material.MetalnessIndex], scaledUV).r * material.MetalnessFactor;
    float roughness = clamp(texture(textures[material.RoughnessIndex], scaledUV).r * material.RoughnessFactor, 0.005, 0.999);
    float ao = texture(textures[material.AOIndex], scaledUV).r * material.AOFactor;

    vec3 V = normalize(Camera.Pos - FragPos);
    vec3 R = reflect(-V, N);
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	
    vec3 Lo = vec3(0.0);
    vec3 ambient = vec3(0.0);

    // Directional Light
    {
        // calculate per-light radiance
        vec3 L = -normalize(Lights.DirectionalLight.Direction);
        vec3 H = normalize(V + L);
        vec3 radiance = Lights.DirectionalLight.Color * Lights.DirectionalLight.Intensity;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	                
            
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // Point Lights
	for(int i = 0; i < Lights.NumPointLights; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(Lights.PointLights[i].Position - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(Lights.PointLights[i].Position - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = Lights.PointLights[i].Color * Lights.PointLights[i].Intensity * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	                
            
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }
    
    // IBL
    vec3 F = vec3(0.0);
    vec3 kS = vec3(0.0);
    vec3 kD = vec3(0.0);
    vec3 specular = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 prefilteredColor = vec3(0.0);
    vec3 irradiance = vec3(0.0);
    vec2 brdf = vec2(0.0);
    {
        F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        
        kS = F;
        kD = 1.0 - kS;
        kD *= 1.0 - metallic;	  
        
        irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse      = irradiance * albedo;
        
        // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
        float MAX_REFLECTION_LOD = textureQueryLevels(prefilterMap) - 1.0;
        prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;    
        brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), clamp(roughness, 0.0, 1.0))).rg;
        specular = prefilteredColor * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * ao;
    }

    vec3 color = Lo + ambient;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

	#ifdef TRANSPARENCY
		float alpha = 1.0;
		if (material.OpacityIndex != UINT32_MAX)
		{
			alpha = texture(textures[material.OpacityIndex], scaledUV).r;
		}
		alpha *= material.Transparency;
		alpha *= material.AlbedoColor.a;

		FragColor = vec4(color, alpha);
	#else
		FragColor = vec4(color, 1.0);
	#endif

    EntityId = Id;
}

