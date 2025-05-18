
#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(push_constant) uniform PushConstantBlock {
    mat4 transform;
} pc;

layout(location = 0) out vec3 vWorldPos;

void main()
{
	vec4 worldPos = pc.transform * vec4(position, 1);
    gl_Position = worldPos;
}

#GEOMETRY
#version 460 core

#define MAX_CASCADES 10

layout(triangles) in;
layout(triangle_strip, max_vertices = 3 * MAX_CASCADES) out;

layout(set = 0, binding = 0) uniform Cascades {
    mat4 cascadeViewProj[MAX_CASCADES];
    vec4 cascadeSplits[MAX_CASCADES];
    uint cascadeCount;
} ubo;

void main() {
    for (int layer = 0; layer < ubo.cascadeCount; ++layer) {
        for (int i = 0; i < 3; ++i) {
            gl_Position = ubo.cascadeViewProj[layer] * gl_in[i].gl_Position;
            gl_Layer = layer;
            EmitVertex();
        }
        EndPrimitive();
    }
}

#FRAGMENT
#version 460 core

void main()
{
    // Depth is written automatically from gl_FragCoord.z
}