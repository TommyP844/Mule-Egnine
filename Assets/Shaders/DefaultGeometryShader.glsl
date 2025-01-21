
#VERTEX
#version 460 core

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = vec4(position, 1);
}

#FRAGMENT
#version 460 core

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(1);
}