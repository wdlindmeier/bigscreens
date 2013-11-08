#version 150 core

out vec4        oColor;
uniform vec4    uColor;

void main( void )
{
	oColor = uColor;//vec4(1.0, 1.0, 1.0, 1.0);
}