#version 150

uniform mat4	uModelViewProjection;
uniform vec2    uTexCoordOffset;
uniform float   uTexScale;
uniform vec4    uColor;

in vec4         vPosition;
out highp vec2	TexCoord;
out lowp vec4	Color;

void main( void )
{
	gl_Position	= uModelViewProjection * vPosition;
	Color       = uColor;
    
    vec2 texCoord = vec2(vPosition.x, (1.0 - vPosition.z));

	TexCoord	= (texCoord * uTexScale) + uTexCoordOffset;
}
