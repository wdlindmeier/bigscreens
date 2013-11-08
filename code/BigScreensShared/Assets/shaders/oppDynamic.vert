#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform samplerBuffer mLastPosition;
uniform samplerBuffer pyramidalTex;
uniform samplerBuffer sphericalTex;
uniform sampler2D	  mNoiseTex;

out vec3 nextPosition;

void main() {
	vec3 mLastPosition = texelFetch( mLastPosition, gl_VertexID );
	vec3 mPyramidalVert = texelFetch( pyramidalTex, gl_VertexID % 6 );
	vec3 mSphericalVert = texelFetch( sphericalTex, gl_VertexID );
	float mPercent = texture( mNoiseTex, ).x
	
}