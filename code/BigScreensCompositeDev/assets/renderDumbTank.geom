#version 150 core
#extension all : warn

layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

uniform float zFactor;
uniform vec3 seperationPoint;

in VS_OUT
{
	vec4 transSeperationPoint;
	float dist;
	bool seperate;
} gs_in[3];

void main()
{
	bool reset = false;
	for( int i = 0; i < 4; i++ ) {
		if( i == 3 ) {
			i = 0;
			reset = true;
		}
			
		if ( gs_in[i].seperate ) {
			vec3 normal = gl_in[i].gl_Position.xyz - gs_in[i].transSeperationPoint.xyz;
			gl_Position = gl_in[i].gl_Position + vec4( zFactor * normal, 1.0);
		
		}
		else
			gl_Position = gl_in[i].gl_Position;
		
		EmitVertex();
		
		if( reset )
			i = 3;
	}
	
	EndPrimitive();
}