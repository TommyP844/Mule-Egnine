#VERTEX
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 color;

layout(push_constant) uniform PushConstantBlock {
    vec2 screenPos;
	vec2 screenSize;
	vec2 viewportSize;
};

void main()
{
	vec2 pos = vec2(position.x, position.z) * 0.5 + 0.5;
	vec2 pixelPos = screenPos + pos * vec2(screenSize.x, -screenSize.y);

    // Convert pixel position to NDC (-1 to 1)
    vec2 ndc = (pixelPos / viewportSize) * 2.0 - 1.0;

    gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable


layout(location = 0) out vec4 Color;

// Bindless Texture Descriptor Set
layout(set = 0, binding = 0) uniform sampler2D textures[4096];


void main()
{
	Color = vec4(1.0);
}

