#version 150 core
#extension all : warn

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
    vec4 color;
} gs_in[3];

out GS_OUT
{
    flat vec4 color[3];
} gs_out;

void main() {
	vec4 v0 = gl_in[0].gl_Position;
	vec4 v1 = gl_in[1].gl_Position;
	vec4 tnorm = normalize(cross(v0, v1));
	
	
}