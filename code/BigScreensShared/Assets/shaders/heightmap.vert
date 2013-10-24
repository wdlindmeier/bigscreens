#version 150

uniform mat4        uModelViewProjection;
uniform sampler2D   uTex0;

in vec4         vPosition;
in vec2         vTexCoord0;
//out highp vec2	TexCoord;
in vec4         vColor;
out lowp vec4	Color;

void main( void )
{
    vec4 pos = vPosition;
    
    // Convert to uv.
    // Assumes the coords are 0-1 scalar
    vec2 heightPosition = vec2(pos.x, 1.0 - pos.y);
    vec4 heightSample = texture(uTex0, heightPosition);
    // xyz should all be the same value if it's a gray image
    pos.z = heightSample.x;
    
	gl_Position	= uModelViewProjection * pos;
	Color = vec4(0.0, 1.0, 0.0, 1.0); //vColor;
}
