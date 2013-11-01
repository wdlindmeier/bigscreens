#version 150 core
#extension all : warn

uniform mat4 projection;
uniform mat4 modelView;
uniform int colorOffset;
uniform float divideNum;

uniform sampler2D heightMap;

layout (location = 0) in vec3 position;

void main(void)
{	
	int modulatedX = (int(position.x) + colorOffset) % 500;
		
	float heightPixel = texture( heightMap, vec2( float(modulatedX) / 500.0 , position.y / 500.0 ) ).x;
	
	if( position.y < 250 ) {
		gl_Position = projection * modelView * vec4( position.x, position.y, position.z + heightPixel * 4, 1.0 );
	}
	else {
		gl_Position = projection * modelView * vec4( position.x, position.y, position.z + heightPixel * 100 * ( (position.y - 250.0) / 250.0), 1.0 );
	}
	
}
