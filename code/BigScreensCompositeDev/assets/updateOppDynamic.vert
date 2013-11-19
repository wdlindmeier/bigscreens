#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform samplerBuffer	pyramidalTex;
uniform samplerBuffer	sphericalTex;
uniform sampler2D		randomMap;
uniform float			percentage;
uniform int				frameNum;

out vec3 outPosition;

void main() {
	percentage;
	vec3 pyramidalPosition = texelFetch( pyramidalTex, gl_VertexID ).xyz;
	vec3 sphericalPosition = texelFetch( sphericalTex, gl_VertexID ).xyz;
	int curVertexId = (gl_VertexID + frameNum) % 400;
	int x = (curVertexId) % 20;
	int y = (curVertexId) / 20;
	vec3 randomPosition = texture( randomMap, vec2( float(x/20.0), float(y/20.0) ) ).xyz;
	vec3 newPos;
	if( randomPosition.r > .7 )
		newPos = pyramidalPosition + ((sphericalPosition - pyramidalPosition) * randomPosition.g) + position;
	else
		newPos = pyramidalPosition;
	outPosition = newPos;
}