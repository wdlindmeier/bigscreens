#version 150 core
#extension all : warn

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexVelocity;
layout (location = 2) in float VertexStartTime;
layout (location = 3) in vec3 VertexInitialVelocity;

out float Transp; // To Fragment Shader

uniform float MinParticleSize;
uniform float MaxParticleSize;

uniform float Time; // Time
uniform float ParticleLifetime; // Particle lifespan

uniform mat4 modelView;
uniform mat4 projection;

void main() {
	float age = Time - VertexStartTime;
	Transp = 0.0;
	gl_Position = projection * modelView * vec4(VertexPosition, 1.0);
	if( Time >= VertexStartTime ) {
		float agePct = age / ParticleLifetime;
		Transp = 1.0 - agePct;
		gl_PointSize = mix( MinParticleSize, MaxParticleSize, agePct );
	}
}