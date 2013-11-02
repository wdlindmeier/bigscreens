#version 150 core
#extension all : warn

layout (lines_adjacency) in;
layout (line_strip, max_vertices = 6) out;

uniform int colorOffset;

void main(void)
{
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    
    gl_Position = gl_in[3].gl_Position;
    EmitVertex();
    
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    
    EndPrimitive();
}
