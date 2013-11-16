#version 150 core
#extension all : warn

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;


layout (location = 1) in bool animating;
layout (location = 2) in float timeLeft;

out vec3 outPosition;
out bool outAnimating;
out float outTimeLeft;

uniform float Time;

uniform samplerBuffer pyramidalTex;
uniform samplerBuffer sphericalTex;
uniform sampler2D	  noiseTex;

uniform float AnimateLifetime;

out VS_OUT_RENDER
{
    vec3 originalPosition;
} vs_out_render;

uniform mat4 projection;
uniform mat4 modelView;
uniform mat4 normalMatrix;

subroutine (RenderPassType)
void update() {
	vec3 pyramidalPosition = texelFetch( pyramidalTex, gl_VertexID ).xyz;
	vec3 sphericalPosition = texelFetch( sphericalTex, gl_VertexID ).xyz;
//	float chance = texture( noiseTex, vec2( (position.x + 50.0) / 100.0 , (position.y + 100.0) / 200.0 ) ).x;
//	
//	if( animating ) {
//		
//	}
//	else {
//		if( chance > 0.8 ) {
//			
//		}
//		else {
//			outPosition = position;
//		}
//	}
}

subroutine (RenderPassType)
void render() {
	vs_out_render.originalPosition = position;
	gl_Position = projection * modelView * vec4(position,1.0);
}

void main() {
	RenderPass();
}