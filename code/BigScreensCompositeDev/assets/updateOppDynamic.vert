#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform samplerBuffer pyramidalTex;
uniform samplerBuffer sphericalTex;
uniform sampler2D	  noiseTex;
uniform float		  percentage;

out vec3 outPosition;

void main() {
	vec3 pyramidalPosition = texelFetch( pyramidalTex, gl_VertexID ).xyz;
	vec3 sphericalPosition = texelFetch( sphericalTex, gl_VertexID ).xyz;
//	float xd = ((sphericalPosition.x - pyramidalPosition.x));
//	float yd = ((sphericalPosition.y - pyramidalPosition.y));
//	float zd = ((sphericalPosition.z - pyramidalPosition.z));
//	float newdistance = sqrt( xd * xd + yd * yd + zd * zd );
	if( percentage < 0.5 )
		outPosition = sphericalPosition;
	else
		outPosition = pyramidalPosition;
//	outPosition = (distance(sphericalPosition - pyramidalPosition) * percentage);
}