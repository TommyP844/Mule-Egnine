#version 450

// Input equirectangular texture
layout(set = 0, binding = 0) uniform sampler2D equirectangularSampler;

// Output cubemap (6 faces)
layout(set = 0, binding = 1, rgba32f) writeonly uniform imageCube cubemapImage;

layout(local_size_x = 16, local_size_y = 16) in;  // Workgroup size

void main() {
    // Get the size of the output cubemap face
    ivec2 cubeSize = imageSize(cubemapImage).xy;

    // Ensure we're within the bounds of the cubemap face
    if (gl_GlobalInvocationID.x >= cubeSize.x || gl_GlobalInvocationID.y >= cubeSize.y) {
        return;
    }

    // Normalized texture coordinates for the current cubemap face pixel
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(cubeSize);

    // Map UV to [-1, 1] range
    uv = uv * 2.0 - 1.0;

    // Determine which cubemap face we're writing to
    int faceIndex = int(gl_GlobalInvocationID.z);  // Assuming the Z dimension is used for face indexing

    // Direction vector based on the current face and UV coordinates
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

    // Normalize the direction vector
    dir = normalize(dir);

    // Convert the direction vector to equirectangular UV coordinates
    float theta = atan(dir.z, dir.x);  // Longitude [-π, π]
    float phi = asin(dir.y);           // Latitude [-π/2, π/2]

    // Convert spherical coordinates to equirectangular UV
    vec2 equirectUV;
    equirectUV.x = (theta + 3.14159265358979) / (2.0 * 3.14159265358979);  // Map to [0, 1]
    equirectUV.y = (phi + 3.14159265358979 / 2.0) / 3.14159265358979;      // Map to [0, 1]

    // Sample the equirectangular texture
    vec4 color = texture(equirectangularSampler, equirectUV);

    // Write the color to the cubemap
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    imageStore(cubemapImage, ivec3(x, y, faceIndex), color);
}