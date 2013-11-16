#version 150 core
#extension all : warn

uniform mat4 projection;
uniform mat4 modelView;
uniform vec2 dimensions;
uniform int colorOffset;
uniform float divideNum;

uniform sampler2D heightMap;

uniform int nearLimit;
uniform int farLimit;
uniform int count;

layout (location = 0) in vec3 position;

void main(void)
{
  // NOTE: use the Triangle Vert Shader so they stay in sync
  /*
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
	*/
}
