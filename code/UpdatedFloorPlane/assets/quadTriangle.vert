#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 modelView;
uniform int colorOffset;

uniform sampler2D heightMap;

uniform int count;
uniform int nearLimit;
uniform int farLimit;

out VS_OUT
{
    vec4 color;
} vs_out;

void main(void)
{
    // NOTE: There are a lot of magic numbers in here.
    // We should make all scale/position/count values uniforms and shader numbers are all unit scalars.
    
    const vec4 colors[] = vec4[](vec4(1.0, 0.5, 0.0, 1.0),
                                 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(1.0, 0.0, 0.0, 1.0),
                                 vec4(1.0, 0.0, 0.0, 1.0),
								 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(0.0, 0.0, 1.0, 1.0),
                                 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(0.0, 0.0, 1.0, 1.0),
								 vec4(1.0, 0.0, 0.0, 1.0),
                                 vec4(1.0, 1.0, 1.0, 1.0),
                                 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(0.0, 1.0, 1.0, 1.0));

	int modulatedX = (int(position.x) + colorOffset) % count;
	float heightPixel = texture( heightMap, vec2( float(modulatedX) / count , position.z / count ) ).x;
    
	if( position.z < farLimit ) {
		// This multiplies the heightPixel (e.g. 0.0-1.0) times a smoothed farlimit starting from the farlimit and incrementally getting bigger towards the count
		gl_Position = projection * modelView * vec4( position.x, position.y - heightPixel * 100 * ( (position.z - farLimit) / (count - farLimit)), position.z, 1.0 );
		vs_out.color = colors[((gl_VertexID + colorOffset) / 4) % 12];
	}
	else if( position.z > nearLimit ) {
		// This multiplies the heightPixel (e.g. 0.0-1.0) times a smoothed nearlimit starting from the nearlimit and incrementally getting bigger towards the count
		gl_Position = projection * modelView * vec4( position.x, position.y + heightPixel * 100 * ( (position.z - nearLimit) / (count - nearLimit)), position.z, 1.0 );
		vs_out.color = colors[((gl_VertexID + colorOffset / 4)) % 12];
	}
	else {
		gl_Position = projection * modelView * vec4( position.x, position.y + heightPixel * 4, position.z, 1.0 );
		vs_out.color = colors[1];
	}
}
