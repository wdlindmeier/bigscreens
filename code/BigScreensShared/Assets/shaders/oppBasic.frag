#version 150 core
#extension all : warn

in vec4 oColor;

out vec4 color;

void main() {
	color = oColor;
}