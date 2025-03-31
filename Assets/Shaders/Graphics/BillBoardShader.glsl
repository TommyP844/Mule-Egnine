#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(location = 0) out vec2 _uv;

struct CameraData
{
    mat4 View;
    mat4 Proj;
	vec3 Pos;
};

layout(binding = 0) uniform CameraBuffer {
    CameraData Camera;
};

layout(push_constant) uniform PC {
    vec4 billboardPos;
    vec4 billboardSize;
};

void main()
{
    _uv = uv;
    vec4 pos = vec4(position.x, position.z, 0, 1);

    vec3 right  = normalize(vec3(Camera.View[0][0], Camera.View[1][0], Camera.View[2][0])); // Right vector
    vec3 up     = normalize(vec3(Camera.View[0][1], Camera.View[1][1], Camera.View[2][1])); // Up vector

    vec3 vertexPos =
        billboardPos.xyz
        + right * pos.x * billboardSize.x
        + up * pos.y * billboardSize.y;

    gl_Position = Camera.Proj * Camera.View * vec4(vertexPos, 1);
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 color;
layout(location = 0) in vec2 uv;

layout(set = 1, binding = 0) uniform sampler2D[] textures;

layout(push_constant) uniform PC2 {
    layout(offset = 32) uint index;  
};

void main()
{
    color = texture(textures[index], uv);
}

