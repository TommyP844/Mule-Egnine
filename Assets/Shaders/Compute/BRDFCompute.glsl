#COMPUTE
#version 450
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform writeonly image2D brdfLUT;

const float PI = 3.14159265359;

vec2 hammersley(uint i, uint N)
{
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return vec2(float(i) /float(N), clamp(rdi, 0.0, 1.0));
}

vec3 importanceSampleGGX(vec2 Xi, float roughness, vec3 N)
{
    float a = roughness * roughness;
    // Sample in spherical coordinates
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    // Construct tangent space vector
    vec3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    // Tangent to world space
    vec3 upVector = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(upVector, N));
    vec3 tangentY = cross(N, tangentX);
    return tangentX * H.x + tangentY * H.y + N * H.z;
}

// From the filament docs. Geometric Shadowing function
// https://google.github.io/filament/Filament.html#toc4.4.2
float V_SmithGGXCorrelated(float NoV, float NoL, float roughness) {
    float a2 = pow(roughness, 4.0);
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

// Karis 2014
vec2 integrateBRDF(float roughness, float NoV)
{
	vec3 V;
    V.x = sqrt(1.0 - NoV * NoV); // sin
    V.y = 0.0;
    V.z = NoV; // cos

    // N points straight upwards for this integration
    const vec3 N = vec3(0.0, 0.0, 1.0);

    float A = 0.0;
    float B = 0.0;
    const uint numSamples = 1024;

    for (uint i = 0u; i < numSamples; i++) {
        vec2 Xi = hammersley(i, numSamples);
        // Sample microfacet direction
        vec3 H = importanceSampleGGX(Xi, roughness, N);

        // Get the light direction
        vec3 L = 2.0 * dot(V, H) * H - V;

        float NoL = clamp(dot(N, L), 0.0, 1.0);
        float NoH = clamp(dot(N, H), 0.0, 1.0);
        float VoH = clamp(dot(V, H), 0.0, 1.0);

        if (NoL > 0.0) {
            // Terms besides V are from the GGX PDF we're dividing by
            float V_pdf = V_SmithGGXCorrelated(NoV, NoL, roughness) * VoH * NoL / NoH;
            float Fc = pow(1.0 - VoH, 5.0);
            A += (1.0 - Fc) * V_pdf;
            B += Fc * V_pdf;
        }
    }

    return 4.0 * vec2(A, B) / float(numSamples);
}

void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = vec2(gl_GlobalInvocationID.xy + 1) / vec2(imageSize(brdfLUT).xy);
    float mu = uv.x;
    float a = uv.y;


    // Output to screen
    vec2 res = integrateBRDF(a, mu);

    // Scale and Bias for F0 (as per Karis 2014)
    imageStore(brdfLUT, ivec2(gl_GlobalInvocationID.xy), vec4(res, 0.0, 0.0));
}