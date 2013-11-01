#version 150 core
#extension all : warn

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 6) out;

uniform bool chooseColor;

in VS_OUT
{
    vec4 color;
} gs_in[4];

out GS_OUT
{
    flat vec4 color[4];
    vec2 uv;
} gs_out;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

int random(int seed, int iterations)
{
    int value = seed;
    int n;
    
    for (n = 0; n < iterations; n++) {
        value = ((value >> 7) ^ (value << 9)) * 15485863;
    }
				
    return value;
}

void main(void)
{
    gl_Position = gl_in[0].gl_Position;
    gs_out.uv = vec2(1.0, 0.0);
    EmitVertex();
    
    gl_Position = gl_in[1].gl_Position;
    gs_out.uv = vec2(0.0, 0.0);
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    gs_out.uv = vec2(0.0, 1.0);
    
    // We're only writing the output color for the last
    // vertex here because they're flat attributes,
    // and the last vertex is the provoking vertex by default
	if( chooseColor ) {
		int r = random(999991929, 4) % 4;
		gs_out.color[0] = gs_in[r].color;
		gs_out.color[1] = gs_in[r].color;
		gs_out.color[2] = gs_in[r].color;
		gs_out.color[3] = gs_in[r].color;
	}
	else {
		gs_out.color[0] = vec4(0.0);
		gs_out.color[1] = vec4(0.0);
		gs_out.color[2] = vec4(0.0);
		gs_out.color[3] = vec4(0.0);
	}
    EmitVertex();
    
    EndPrimitive();
    
    gl_Position = gl_in[0].gl_Position;
    gs_out.uv = vec2(1.0, 0.0);
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    gs_out.uv = vec2(0.0, 1.0);
    EmitVertex();
    
    gl_Position = gl_in[3].gl_Position;
    gs_out.uv = vec2(1.0, 1.0);
	
	if( chooseColor ) {
		int r = random(999991929, 4) % 4;
		gs_out.color[0] = gs_in[r].color;
		gs_out.color[1] = gs_in[r].color;
		gs_out.color[2] = gs_in[r].color;
		gs_out.color[3] = gs_in[r].color;
	}
	else {
		gs_out.color[0] = vec4(0.0);
		gs_out.color[1] = vec4(0.0);
		gs_out.color[2] = vec4(0.0);
		gs_out.color[3] = vec4(0.0);
	}
    EmitVertex();
    
    EndPrimitive();
}
