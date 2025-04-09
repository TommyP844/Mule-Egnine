
#SETUP

CullMode = Back
DepthTest = False 
DepthWrite = False
Attachment = { Location = 0, Format = RGBA32F, BlendEnable = False }
Attachment = { Location = 1, Format = RG32UI, BlendEnable = False }
Attachment = { Location = 2, Format = R32F, BlendEnable = False }
DepthFormat = D32F

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

struct CameraData
{
    mat4 View;
    mat4 Proj;
	vec3 Pos;
};

layout(binding = 0) uniform CameraBuffer {
    CameraData Camera;
};

void main()
{
	gl_Position = Camera.Proj * Camera.View * transform * vec4(position, 1);
}

#FRAGMENT
#version 460 core

layout(location = 2) out float Entity;


void main()
{
	Entity = 1.0;
}