#version 150 core
#extension all : warn

in GS_OUT
{
    flat vec4 color[4];
    vec2 uv;
    float fog;
    
} fs_in;

out vec4 color;
//out float fog;

void main(void)
{
    float brightness = 1.0 - (fs_in.fog * 0.000075);

    vec4 c1 = mix(fs_in.color[0], fs_in.color[1], fs_in.uv.x);
    vec4 c2 = mix(fs_in.color[2], fs_in.color[3], fs_in.uv.x);

    color = mix(c1, c2, fs_in.uv.y) * brightness;
}
