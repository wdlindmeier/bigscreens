#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 modelView;
uniform vec2 dimensions;
uniform int colorOffset;

uniform sampler2D heightMap;

uniform int count;
uniform int nearLimit;
uniform int farLimit;
uniform float fft[64]; // 64 == num fft channels

out VS_OUT
{
    vec4 color;
    float fog;
    
} vs_out;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(void)
{    
    // Normalized.
    // NOTE: All scale and positioning is now handled by the matrix.
    // The vectors exist in 0-1 space now.
    float textureHeight = texture( heightMap, vec2(position.x, position.z) ).x;
    gl_Position = projection * modelView * vec4(position.x,
                                                position.y + textureHeight + 0.01, // +0.01 to raise above the triangles
                                                position.z,
                                                1.0 );
    
    vec4 modelPosition = projection * vec4(position, 1.0);
    
    float scalarPosX = modelPosition.x;
    float scalarPosY = modelPosition.y;
    float scalarPosZ = modelPosition.z;
    
    float wrappingPosX = abs(1.0 - (scalarPosX));
    float wrappingPosZ = abs(1.0 - (scalarPosZ));

    float spectrumScalar = wrappingPosX + wrappingPosZ;
    float frameScalar = float(colorOffset % 1000) / 1000.0;
    
    // Color based on position
    vec3 rgbColor = hsv2rgb(vec3(spectrumScalar, 1.0, 1.0));
    
    vec4 positionColor = vec4(rgbColor.rgb, 1.0);
    
    int fftBand = int(64.0 * spectrumScalar) % 64;
    
    float volume = fft[fftBand];
    
    // Boost volume at the end of the spectrum
    float scalarSpectrum = float(fftBand) / 64.0;
    volume *= 1.0 + (scalarSpectrum * 3);
    volume *= 0.25;
    
    float fftHeightMulti = 1.0;
    vec4 pickedColor = positionColor;

    vs_out.color = vec4(pickedColor.rgb, 0.25);

    vs_out.fog = gl_Position.z;
}
