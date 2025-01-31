
#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstantBlock {
    mat4 transform;
} pc;

void main()
{
	gl_Position = ubo.proj * ubo.view * pc.transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 1) uniform sampler2D textures[];

layout(binding = 2) uniform UniformBufferObject {
    uint albedoImage;
} ubo;

layout(push_constant) uniform PushConstantBlock {
    uint materialIndex;
} pc;

void main()
{
	fragColor = vec4(1);
}