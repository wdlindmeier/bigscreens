#version 150 core
#extension all: warn

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 modelView;

out VS_OUT
{
	vec3 originalPosition;
} vs_out;

void main() {
	vs_out.originalPosition = position;
	gl_Position = projection * modelView * vec4(position,1.0);
}