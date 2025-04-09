#COMPUTE
#version 450

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) writeonly uniform image2D outputImage;
layout(binding = 1) uniform sampler2D inputImage;

void main() {
    ivec2 texSize = textureSize(inputImage, 0);
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    
    if (coord.x >= texSize.x || coord.y >= texSize.y) {
        return;
    }
    
    float kernelX[3][3] = float[3][3](
        float[3](-1, 0, 1),
        float[3](-2, 0, 2),
        float[3](-1, 0, 1)
    );
    
    float kernelY[3][3] = float[3][3](
        float[3](-1, -2, -1),
        float[3](0, 0, 0),
        float[3](1, 2, 1)
    );
    
    float sumX = 0.0;
    float sumY = 0.0;
    
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            ivec2 sampleCoord = clamp(coord + ivec2(i, j), ivec2(0), texSize - ivec2(1));
            float intensity = texelFetch(inputImage, sampleCoord, 0).r;
            sumX += intensity * kernelX[i + 1][j + 1];
            sumY += intensity * kernelY[i + 1][j + 1];
        }
    }
    
    float edgeStrength = length(vec2(sumX, sumY));
    
    // Only draw the outline if the object is detected (non-zero intensity)
    float originalIntensity = texelFetch(inputImage, coord, 0).r;
    if(originalIntensity == 0.0 && edgeStrength > 0.0)
    {
        vec3 color = texelFetch(inputImage, coord, 0).rgb;
        imageStore(outputImage, coord, vec4(edgeStrength, 0.0, 0.0, 1.0));
    }
}