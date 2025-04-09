#SETUP

CullMode = None
DepthTest = False 
DepthWrite = False
Attachment = { Location = 0, Format = RGBA32F, BlendEnable = False }
DepthFormat = D32F


#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

struct CameraData
{
    mat4 View;
    mat4 Proj;
	vec3 Pos;
};

layout(binding = 0) uniform CameraBuffer {
    CameraData Camera;
};

layout(push_constant) uniform PushConstantBlock {
    mat4 transform;
};

void main()
{
	gl_Position = Camera.Proj * Camera.View * transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core

layout(location = 0) out vec4 FragColor;

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(push_constant) uniform PushConstantBlock {
    layout(offset = 64) vec3 color;
};

void main()
{
    FragColor = vec4(color, 1);
}

