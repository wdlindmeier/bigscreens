#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec4 oColor;

uniform mat4 projection;
uniform mat4 modelView;

void main()
{	
    gl_Position = projection * modelView * vec4(position,1.0);
	oColor = vec4(1.0, 0.07, 0.57, 1.0);
}