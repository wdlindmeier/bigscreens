#version 150 core
#extension all : warn

in vec3 oTexCoord;
layout (location = 0) out vec4 FragColor;

uniform BlobSettings {
	vec4 InnerColor;
	vec4 OuterColor;
	float RadiusInner;
	float RadiusOuter;
};

void main() {
	float dx = oTexCoord.x - 0.5;
	float dy = oTexCoord.y - 0.5;
	float dist = sqrt(dx * dx + dy * dy);
	FragColor = mix( InnerColor, OuterColor, smoothstep( RadiusInner, RadiusOuter, dist ) );
}