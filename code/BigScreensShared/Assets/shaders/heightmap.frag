#version 150

out vec4            oColor;
in vec4             Color;
// in vec2             TexCoord;

void main( void )
{
	oColor = Color;
}