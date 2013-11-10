#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 modelView;
uniform int colorOffset;
uniform float divideNum;

uniform sampler2D heightMap;

out VS_OUT
{
    vec4 color;
	float randValue;
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

	int modulatedX = (int(position.x) + colorOffset) % 500;
	float heightPixel = texture( heightMap, vec2( float(modulatedX) / 500.0 , position.z / 500.0 ) ).x;
	
	if( position.z < 250 ) {
		gl_Position = projection * modelView * vec4( position.x, position.y + heightPixel * 20, position.z, 1.0 );
		vs_out.color = colors[1];
	}
	else {
		gl_Position = projection * modelView * vec4( position.x, position.y + heightPixel * 100 * ( (position.z - 250.0) / 250.0 ), position.z, 1.0 );
		vs_out.color = colors[(gl_VertexID + colorOffset / 4) % 12];
	}
    
    // TMP
    // vs_out.color = vec4(1.0,1.0,0.0,0.5);
}
