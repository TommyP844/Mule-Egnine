#version 450 core
#extension GL_EXT_samplerless_texture_functions : require

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(binding = 0) uniform samplerCube envMap;        // Cube map environment texture
layout(binding = 1, rgba16f) uniform writeonly imageCube irradianceMap;  // Output irradiance map

const float PI = 3.14159265359;

// Integrate over hemisphere to get irradiance
void main() {
    // Get the texture size for irradiance map
    ivec2 cubeSize = imageSize(irradianceMap); // This is a 3D texture (cube)
    ivec3 texCoord = ivec3(gl_GlobalInvocationID.xyz);

    // Make sure we're within bounds
    if (texCoord.x > cubeSize.x || texCoord.y > cubeSize.y)
        return;

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
            dir = vec3(uv.x, -1.0, -uv.y);
            break;
        case 3:  // -Y
            dir = vec3(uv.x, 1.0, uv.y);
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
    vec3 irradiance = vec3(0.0);  

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, dir));
    up         = normalize(cross(dir, right));

    float sampleDelta = 0.05;
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * dir; 
    
            irradiance += texture(envMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));


    // Store the result in the irradiance map
    texCoord.y = cubeSize.y - texCoord.y;
    imageStore(irradianceMap, texCoord, vec4(irradiance, 1.0));
}
