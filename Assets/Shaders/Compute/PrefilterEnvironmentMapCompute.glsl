#version 450 core
#extension GL_EXT_samplerless_texture_functions : require

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding = 0) uniform samplerCube envMap;        // Cube map environment texture
layout(binding = 1, rgba16f) uniform imageCube prefilteredEnvMap;  // Output prefiltered cubemap

const float PI = 3.14159265359;
const int SAMPLE_COUNT = 2048; // Number of samples to approximate the reflection

// Function to sample a direction using importance sampling for a given roughness
vec3 sampleHemisphere(float roughness, vec3 normal) {
    // Importance sampling based on roughness
    float phi = 2.0 * PI * fract(sin(dot(normal, normal)) * 43758.5453);
    float cosTheta = pow(fract(sin(dot(normal, normal)) * 43758.5453), 1.0 / (roughness + 1.0));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 tangent = normalize(cross(vec3(0.0, 1.0, 0.0), normal));
    vec3 bitangent = cross(normal, tangent);

    vec3 sampleDir = cosTheta * normal + sinTheta * (cos(phi) * tangent + sin(phi) * bitangent);

    return normalize(sampleDir);
}

// Generate prefiltered cubemap for specular IBL
void main() {
    // Get the texture size for prefiltered environment map (this is a 3D texture cube)
    ivec2 texSize = imageSize(prefilteredEnvMap); // This is a 3D texture (cube)
    ivec3 texCoord = ivec3(gl_GlobalInvocationID.xyz);

    // Ensure that we're within bounds for all three dimensions
    if (texCoord.x >= texSize.x || texCoord.y >= texSize.y)
        return;

    // Roughness value is based on the Z dimension (the mipmap level is roughness)
    float roughness = 1.f;

    // Direction vector (same as the irradiance map generation)
    vec2 uv = vec2(texCoord.x, texCoord.y) / vec2(texSize.x, texSize.y);
    uv = uv * 2.0 - 1.0;

    int faceIndex = texCoord.z;  // Z dimension for face indexing
    vec3 dir;
    switch (faceIndex) {
        case 0:  // +X
            dir = vec3(1.0, -uv.y, -uv.x);
            break;
        case 1:  // -X
            dir = vec3(-1.0, -uv.y, uv.x);
            break;
        case 2:  // +Y
            dir = vec3(uv.x, 1.0, uv.y);
            break;
        case 3:  // -Y
            dir = vec3(uv.x, -1.0, -uv.y);
            break;
        case 4:  // +Z
            dir = vec3(uv.x, -uv.y, 1.0);
            break;
        case 5:  // -Z
            dir = vec3(-uv.x, -uv.y, -1.0);
            break;
    }

    dir = normalize(dir);

    // We will pre-filter by sampling in the hemisphere around the normal
    vec3 color = vec3(0.0);
    uint sampleCount = 0;

    // Use importance sampling to create more samples in the reflection direction
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        // Sample a random direction in the hemisphere
        vec3 sampleDir = sampleHemisphere(roughness, dir);

        // Sample the environment map at this direction
        vec3 envColor = texture(envMap, sampleDir).rgb;

        // Lambertian reflection model (Cosine-weighted)
        float cosTheta = max(dot(sampleDir, dir), 0.0);

        color += envColor * cosTheta;
        sampleCount++;
    }

    // Normalize by the number of samples and apply PI factor
    if (sampleCount > 0) {
        color /= float(sampleCount);
        color *= PI;
    }

    // Store the result in the prefiltered environment map (cubemap)
    imageStore(prefilteredEnvMap, texCoord, vec4(color, 1.0));
}
