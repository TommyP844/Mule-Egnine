#version 460 core

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba16f) uniform image2D brdfLUT;

const float PI = 3.14159265359;

// Schlick-GGX Geometry Function
float GeometrySchlickGGX(float NdotV, float roughness) {
    float a = roughness * roughness;
    float k = (a * a) / 2.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Smith's Combined GGX Geometry Function
float GeometrySmith(float NdotV, float NdotL, float roughness) {
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

// Importance Sampling GGX
vec2 IntegrateBRDF(float NdotV, float roughness) {
    vec3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);  // sin(theta)
    V.y = 0.0;
    V.z = NdotV;                      // cos(theta)

    float A = 0.0;
    float B = 0.0;
    const uint SAMPLE_COUNT = 1024;

    for (uint i = 0; i < SAMPLE_COUNT; i++) {
        float Xi1 = float(i) / float(SAMPLE_COUNT);
        float Xi2 = fract(265.34 * float(i)); // Quasi-random sequence

        float a = roughness * roughness;
        float phi = 2.0 * PI * Xi1;
        float cosTheta = sqrt((1.0 - Xi2) / (1.0 + (a * a - 1.0) * Xi2));
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0) {
            float G = GeometrySmith(NdotV, NdotL, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    return vec2(A, B) / float(SAMPLE_COUNT);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 texelSize = 1.0 / vec2(imageSize(brdfLUT));
    
    vec2 uv = vec2(texelCoord) * texelSize;
    vec2 brdf = IntegrateBRDF(uv.x, uv.y);
    
    imageStore(brdfLUT, texelCoord, vec4(brdf, 0.0, 1.0));
}
