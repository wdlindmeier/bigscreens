#version 150 core
#extension all : warn

out vec4 color;
uniform vec4 uColor;

void main(void)
{
    color = uColor;//vec4(1.0);
}
