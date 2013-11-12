#version 150 core
#extension all : warn

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 6) out;

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

uniform mat4 modelView;
uniform mat4 normalMatrix;
uniform vec3 lightPosition;


in vec3 outPosition[4];
in bool outAnimating[4];
in float outTimeLeft[4];

out vec3 position[4];
out bool animating[4];
out float timeLeft[4];

in VS_OUT_RENDER
{
    vec3 originalPosition;
} gs_in_render[4];

out GS_OUT
{
    flat vec4 color[4];
} gs_out;

const vec4 diffusePink = vec4(1.0, 0.07, 0.570, 1.0); //PINK
const vec4 ambientPink = vec4(0.2, 0.00, 0.014, 1.0);


vec4 ambientDiffuse( vec3 v0, vec3 v1, vec3 v2 )
{
	vec3 ab = v1 - v0;
	vec3 ac = v2 - v0;
	vec3 normal = normalize(cross(ab, ac));
	
	// Calculate the transformed face normal and the view direction vector
	vec3 transformed_normal = (vec4(normal, 0.0) * modelView).xyz;
	
	// Take the dot product of the normal with the view direction
	float d = dot(lightPosition, normal);
	
	return ambientPink + ( diffusePink * max( d, 0.0 ) );
}

subroutine (RenderPassType)
void update() {
	
}

subroutine (RenderPassType)
void render() {
	vec4 oColor1 = ambientDiffuse( gl_in[0].gl_Position.xyz, gl_in[3].gl_Position.xyz, gl_in[1].gl_Position.xyz );
	
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[3].gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[1].gl_Position;
	gs_out.color[0] = oColor1;
	gs_out.color[1] = oColor1;
	gs_out.color[2] = oColor1;
	gs_out.color[3] = oColor1;
	EmitVertex();
	
	EndPrimitive();
	
	vec4 oColor2 = ambientDiffuse( gl_in[1].gl_Position.xyz, gl_in[3].gl_Position.xyz, gl_in[2].gl_Position.xyz );
	
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[3].gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[2].gl_Position;
	gs_out.color[0] = oColor2;
	gs_out.color[1] = oColor2;
	gs_out.color[2] = oColor2;
	gs_out.color[3] = oColor2;
	EmitVertex();
	
	EndPrimitive();
}

void main() {
	
	RenderPass();
}