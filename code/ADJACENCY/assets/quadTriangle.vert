#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 modelView;
uniform int vid_offset = 3;

out VS_OUT
{
    vec4 color;
} vs_out;

void main(void)
{
    
    const vec4 colors[] = vec4[](vec4(1.0, 0.5, 0.0, 1.0),
                                 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(0.0, 1.0, 1.0, 1.0));
    
    gl_Position = projection * modelView * vec4(position, 1.0);
    vs_out.color = colors[(gl_VertexID + vid_offset) % 4];
}
