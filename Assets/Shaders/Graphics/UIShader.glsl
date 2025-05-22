#VERTEX
#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;
layout(location = 3) in uint textureIndex;
layout(location = 4) in uint isFont;


layout(location = 0) out vec2 _uv;
layout(location = 1) out vec4 _color;
layout(location = 2) out flat uint _textureIndex;
layout(location = 3) out flat uint _isFont;

layout(push_constant) uniform PushConstantBlock {
	vec2 viewportSize;
};

void main()
{
    _uv = uv;
    _color = color;
    _textureIndex = textureIndex;
    _isFont = isFont;
	vec2 pos = vec2(position.x / viewportSize.x, (viewportSize.y - position.y) / viewportSize.y);

    // Convert pixel position to NDC (-1 to 1)
    vec2 ndc = pos * 2.0 - 1.0;

    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);
}

#FRAGMENT
#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable


layout(location = 0) out vec4 FragColor;

// Bindless Texture Descriptor Set
layout(set = 0, binding = 0) uniform sampler2D textures[4096];

layout(location = 0) in vec2 _uv;
layout(location = 1) in vec4 _color;
layout(location = 2) in flat uint _textureIndex;
layout(location = 3) in flat uint _isFont;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
	 vec4 texColor = texture(textures[_textureIndex], _uv);

    if (_isFont == 1) {
        // SDF font rendering
        vec3 sdf = texColor.rgb;

        // Compute the signed median of the distances
        float sd = median(sdf.r, sdf.g, sdf.b);
        
        // Screen-space smoothing based on pixel derivative
        float smoothing = fwidth(sd) * 0.5;
        
        // Use smoothstep for anti-aliasing based on signed distance
        float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, sd);
        
        // Output color
        FragColor = vec4(_color.rgb, alpha * _color.a);
    } else {
        // Regular image rendering
        FragColor = vec4(vec3(texColor * _color), _color.a);
    }
}

