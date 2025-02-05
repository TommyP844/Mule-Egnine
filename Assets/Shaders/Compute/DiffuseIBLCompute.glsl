#version 450 core
#extension GL_EXT_samplerless_texture_functions : require

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding = 0) uniform samplerCube envMap;        // Cube map environment texture
layout(binding = 1, rgba16f) uniform imageCube irradianceMap;  // Output irradiance map

const float PI = 3.14159265359;

// Integrate over hemisphere to get irradiance
void main() {
    // Get the texture size for irradiance map
    ivec2 cubeSize = imageSize(irradianceMap); // This is a 3D texture (cube)
    ivec3 texCoord = ivec3(gl_GlobalInvocationID.xyz);

    // Make sure we're within bounds
    if (texCoord.x >= cubeSize.x || texCoord.y >= cubeSize.y)
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
    vec3 irradiance = vec3(0);
    uint sampleCount = 0;
    // Sample the hemisphere around the normal (using spherical coordinates)
    for (float phi = 0; phi < 360; phi += 0.5) {
        for (float theta = 67.5; theta < 112.5; theta += 0.5) {
            // Convert spherical to cartesian coordinates for sampling direction
            float phiRad = radians(phi);
            float thetaRad = radians(theta);
            vec3 sampleVec = vec3(
                sin(thetaRad) * cos(phiRad),
                cos(thetaRad),
                sin(thetaRad) * sin(phiRad)
            );

            mat3 rotationMatrix = mat3(
                cross(vec3(0.0, 1.0, 0.0), dir),  // X-axis rotation
                cross(vec3(1.0, 0.0, 0.0), dir),  // Y-axis rotation
                dir                                    // Z-axis is the normal
            );

            sampleVec = normalize(rotationMatrix * sampleVec);

            // Cosine-weighted sample (Lambertian reflection model)
            float cosTheta = max(dot(sampleVec, dir), 0.0); // Cosine of angle between normal and light direction
            if (cosTheta > 0.0) {
                // Sample the environment map at this direction (the sampleVec is a 3D direction)
                vec3 envColor = texture(envMap, sampleVec).rgb;

                // Accumulate the irradiance with cosine-weighted contribution
                irradiance += envColor * cosTheta;
                sampleCount++;
            }
        }
    }

    // Avoid division by zero
    if (sampleCount > 0) {
        irradiance /= float(sampleCount); // Normalize by the number of samples
        irradiance *= PI; // Multiply by PI for correct irradiance scaling
    }


    // Store the result in the irradiance map
    imageStore(irradianceMap, texCoord, vec4(irradiance, 1.0));
}
