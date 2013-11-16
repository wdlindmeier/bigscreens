#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 oColor;

uniform mat4 projection;
uniform mat4 modelView;
uniform mat3 normalMatrix;
uniform vec3 lightPosition;
uniform vec4 mColor;

const vec3 yellow = vec3(1.0, 1.0, 0.0);

vec3 ad( vec4 eyePositionVertex, vec3 norm ) {
	vec3 v = normalize(vec3(-position));
	return (( 0.2 * mColor ) + ( mColor * max( dot(lightPosition, norm), 0.0 ) * 0.8 )).xyz ;
}

void main() {
	
	vec3 eyeNorm = normalize( normalMatrix * normal );
	vec4 eyePosition = modelView * vec4( position, 1.0 );
	
			 //AMBIENT			//DIFFUSE
	oColor = ad(eyePosition, eyeNorm);
	
	gl_Position = projection * modelView * vec4(position, 1.0);
}