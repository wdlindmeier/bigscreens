#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 texCoord;

out vec3 oTexCoord;

void main() {
	oTexCoord = texCoord;
	gl_Position = vec4(position, 1.0);
}