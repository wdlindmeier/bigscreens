#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 modelView;

void main()
{
	gl_Position = projection * modelView * vec4( position, 1.0 );
}