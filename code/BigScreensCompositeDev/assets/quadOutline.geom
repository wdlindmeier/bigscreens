#version 150 core
#extension all : warn

layout (lines_adjacency) in;
layout (line_strip, max_vertices = 6) out;

uniform int colorOffset;

in VS_OUT
{
    vec4 color;
    float fog;
    
} gs_in[4];

out GS_OUT
{
    float fog;
    vec4 color;
    
} gs_out;

void main(void)
{
    gl_Position = gl_in[0].gl_Position;
    gs_out.fog = gs_in[0].fog;
    gs_out.color = gs_in[0].color;
    EmitVertex();
    
    gl_Position = gl_in[1].gl_Position;
    gs_out.fog = gs_in[1].fog;
    gs_out.color = gs_in[1].color;
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    gs_out.fog = gs_in[2].fog;
    gs_out.color = gs_in[2].color;
    EmitVertex();
    
    gl_Position = gl_in[3].gl_Position;
    gs_out.fog = gs_in[3].fog;
    gs_out.color = gs_in[3].color;
    EmitVertex();
    
    gl_Position = gl_in[0].gl_Position;
    gs_out.fog = gs_in[0].fog;
    gs_out.color = gs_in[0].color;
    EmitVertex();
    
    EndPrimitive();
}
