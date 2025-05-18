#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 uv;
layout(location = 4) in vec3 color;

layout(location = 0) out vec3 pos;

struct CameraData
{
    mat4 ViewProj;
    mat4 View;
	mat4 Proj;
	vec3 Pos;
	vec3 ViewDir;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    CameraData Camera;
};


void main()
{
	pos = position;
	gl_Position = Camera.Proj * mat4(mat3(Camera.View)) * vec4(position, 1.0);
	gl_Position.z = gl_Position.w; // Set depth to the far plane
}

#FRAGMENT
#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 0) out vec4 fragColor;

layout(set = 1, binding = 0) uniform samplerCube cubeMap;

void main()
{
	vec3 samplePos = pos;
	fragColor = vec4(texture(cubeMap, normalize(samplePos)).xyz, 1);
}