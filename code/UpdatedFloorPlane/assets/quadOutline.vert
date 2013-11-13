#version 150 core
#extension all : warn

uniform mat4 projection;
uniform mat4 modelView;
uniform int colorOffset;
uniform float divideNum;

uniform sampler2D heightMap;

uniform int nearLimit;
uniform int farLimit;
uniform int count;

layout (location = 0) in vec3 position;

void main(void)
{
    // NOTE: There are a lot of magic numbers in here.
    // We should make all scale/position/count values uniforms and shader numbers are all unit scalars.
	nearLimit;
	int modulatedX = (int(position.x) + colorOffset) % count;
		
	float heightPixel = texture( heightMap, vec2( float(modulatedX) / count, position.z / count ) ).x;
	
	if( position.z < farLimit ) {
		gl_Position = projection * modelView * vec4( position.x, position.y - heightPixel * 100 * ( (position.z - farLimit) / (count - farLimit)), position.z, 1.0 );
	}
	else if( position.z > nearLimit ) {
		gl_Position = projection * modelView * vec4( position.x, position.y + heightPixel * 100 * ( (position.z - nearLimit) / (count - nearLimit)), position.z, 1.0 );
	}
	else {
		gl_Position = projection * modelView * vec4( position.x, position.y + heightPixel * 4, position.z, 1.0 );
	}
	
}
