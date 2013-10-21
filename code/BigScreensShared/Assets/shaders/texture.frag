#version 150 core

out vec4 oColor;
in uniform sampler2D texture;

void main( void )
{
    vec2 position = ( gl_FragCoord.xy );
	oColor = texture2D(texture, position);
}