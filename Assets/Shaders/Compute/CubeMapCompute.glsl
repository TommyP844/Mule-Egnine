#version 450

// Input equirectangular texture
layout(set = 0, binding = 0) uniform sampler2D equirectangularSampler;

// Output cubemap (6 faces)
layout(set = 0, binding = 1) writeonly uniform imageCube cubemapImage;

layout(local_size_x = 16, local_size_y = 16) in;  // Workgroup size (you can adjust this)

void main() {
    // Get the current pixel position in the equirectangular texture
    ivec2 texSize = textureSize(equirectangularSampler, 0);  // Size of the equirectangular map
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);  // Current pixel index

    if (pixel.x >= texSize.x || pixel.y >= texSize.y) {
        return;  // Prevent out-of-bounds access
    }

    // Convert pixel position to spherical coordinates
    float theta = (float(pixel.x) / float(texSize.x)) * 2.0 * 3.14159265358979;  // Longitude [0, 2?]
    float phi = (float(pixel.y) / float(texSize.y)) * 3.14159265358979 - 3.14159265358979 / 2.0;  // Latitude [-?/2, ?/2]

    // Convert to Cartesian coordinates (direction vector)
    float x_dir = cos(phi) * sin(theta);
    float y_dir = sin(phi);
    float z_dir = cos(phi) * cos(theta);

    // Compute which cubemap face this direction corresponds to
    int faceIndex = 0;
    vec3 cubemapCoord = vec3(0.0);

    if (abs(x_dir) > abs(y_dir) && abs(x_dir) > abs(z_dir)) {
        // X face
        if (x_dir > 0.0) {
            // Positive X
            faceIndex = 0;
            cubemapCoord = vec3(1.0, -z_dir, -y_dir);
        } else {
            // Negative X
            faceIndex = 1;
            cubemapCoord = vec3(-1.0, -z_dir, y_dir);
        }
    } else if (abs(y_dir) > abs(x_dir) && abs(y_dir) > abs(z_dir)) {
        // Y face
        if (y_dir > 0.0) {
            // Positive Y
            faceIndex = 2;
            cubemapCoord = vec3(x_dir, 1.0, z_dir);
        } else {
            // Negative Y
            faceIndex = 3;
            cubemapCoord = vec3(x_dir, -1.0, -z_dir);
        }
    } else {
        // Z face
        if (z_dir > 0.0) {
            // Positive Z
            faceIndex = 4;
            cubemapCoord = vec3(x_dir, -y_dir, 1.0);
        } else {
            // Negative Z
            faceIndex = 5;
            cubemapCoord = vec3(x_dir, y_dir, -1.0);
        }
    }

    // Normalize cubemap coordinates
    cubemapCoord = normalize(cubemapCoord);

    // Convert direction vector to cubemap texture coordinates [0, 1] range
    float u = (cubemapCoord.x + 1.0) * 0.5;
    float v = (cubemapCoord.y + 1.0) * 0.5;

    // Fetch texel color from the equirectangular texture
    vec4 texelColor = texelFetch(equirectangularSampler, pixel, 0);

    // Write the texel to the appropriate cubemap face
    imageStore(cubemapImage, ivec3(gl_GlobalInvocationID.xyz), texelColor);
}
