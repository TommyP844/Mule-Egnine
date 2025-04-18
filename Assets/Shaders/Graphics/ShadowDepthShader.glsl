
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

void main()
{
	gl_Position = pc.transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core


void main()
{
}