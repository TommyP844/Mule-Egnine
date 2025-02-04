
#VERTEX
#version 460 core

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 pos;

layout(binding = 1) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;


void main()
{
	pos = position;
	gl_Position = ubo.proj * mat4(mat3(ubo.view)) * vec4(position, 1.0);
	gl_Position.z = gl_Position.w; // Set depth to the far plane
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 pos;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform samplerCube cubeMap;


void main()
{
	fragColor = vec4(texture(cubeMap, normalize(pos)).xyz, 1);
}