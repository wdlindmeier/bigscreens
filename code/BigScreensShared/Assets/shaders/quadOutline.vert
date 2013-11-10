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
    // NOTE: There are a lot of magic numbers in here.
    // We should make all scale/position/count values uniforms and shader numbers are all unit scalars.

	int modulatedX = (int(position.x) + colorOffset) % 500;
		
	float heightPixel = texture( heightMap, vec2( float(modulatedX) / 500.0 , position.z / 500.0 ) ).x;
	
	if( position.z < 250 ) {
		gl_Position = projection * modelView * vec4( position.x, position.y + heightPixel * 4, position.z, 1.0 );
	}
	else {
		gl_Position = projection * modelView * vec4( position.x, position.y + heightPixel * 100 * ( (position.z - 250.0) / 250.0), position.z, 1.0 );
	}
	
}
