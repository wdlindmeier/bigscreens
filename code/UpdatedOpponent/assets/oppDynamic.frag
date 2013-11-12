#version 150 core
#extension all : warn

in GS_OUT
{
    flat vec4 color[4];
} gs_out;

out vec4 color;

void main() {
	color = gs_out.color[0];
}