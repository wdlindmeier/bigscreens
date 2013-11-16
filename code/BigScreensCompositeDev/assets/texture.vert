#version 150

uniform mat4	uModelViewProjection;
uniform vec4    uColor;

in vec4         vPosition;
in vec2         vTexCoord0;
out highp vec2	TexCoord;
out lowp vec4	Color;

void main( void )
{
	gl_Position	= uModelViewProjection * vPosition;
	Color       = uColor;
	TexCoord	= vTexCoord0;
}
