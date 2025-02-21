
#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(push_constant) uniform PushConstantBlock {
    mat4 transform;
};


void main()
{
	gl_Position = transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core

layout(location = 1) out float Entity;
layout(location = 2) out uint EntityId;

layout(push_constant) uniform PushConstantBlock {
    layout(offset = 64) uint id;
};

void main()
{
	Entity = 1.0;
	EntityId = id;
}