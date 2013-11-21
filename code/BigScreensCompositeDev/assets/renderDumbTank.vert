#version 150 core
#extension all : warn

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 tankOriginal;

uniform mat4 modelView;
uniform mat4 projection;
uniform vec3 seperationPoint;
uniform vec3 tankPosition;
uniform vec3 tankVector;

out VS_OUT
{
	vec4 transSeperationPoint;
	float dist;
	bool seperate;
} vs_out;

void main()
{
	gl_Position	= projection * modelView * position;
	vs_out.transSeperationPoint = projection * modelView * vec4(seperationPoint, 1.0);
	
	float xd = position.x - seperationPoint.x;
	float yd = position.y - seperationPoint.y;
	float zd = position.z - seperationPoint.z;
	float dist = sqrt( xd * xd + yd * yd + zd * zd );
	
	if( dist < 200.0 ) {
		vs_out.seperate = true;
		vs_out.dist = dist;
	}
	else {
		vs_out.seperate = false;
		vs_out.dist = 0.0;
	}
	
}