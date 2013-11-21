#version 150 core
#extension all : warn

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 modelView;
uniform vec2 dimensions;
uniform vec3 tankPosition;
uniform vec3 tankVector;
uniform int colorOffset;

uniform vec3 groundScale;
uniform vec3 groundOffset;
uniform float mountainMultiplier;
uniform float alphaMultiplier;

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
    const vec4 colors[] = vec4[](vec4(1.0, 0.5, 0.0, 1.0),
                                 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(1.0, 0.0, 0.0, 1.0),
                                 vec4(1.0, 0.0, 0.0, 1.0),
								 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(0.0, 0.0, 1.0, 1.0),
                                 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(0.0, 0.0, 1.0, 1.0),
								 vec4(1.0, 0.0, 0.0, 1.0),
                                 vec4(1.0, 1.0, 1.0, 1.0),
                                 vec4(0.0, 0.0, 0.0, 1.0),
                                 vec4(0.0, 1.0, 1.0, 1.0));
    
    vec4 modelPosition = modelView * vec4(position, 1.0);
    vec4 projectionPosition = projection * vec4(position, 1.0);

    float scalarPosX = position.x;
    float scalarPosY = position.y;
    float scalarPosZ = position.z;
    
    float wrappingPosX = abs(1.0 - (scalarPosX));
    float wrappingPosZ = abs(1.0 - (scalarPosZ));
    float spectrumScalar = wrappingPosX + wrappingPosZ;
    
    // Color based on position
    // vec3 rgbColor = hsv2rgb(vec3(spectrumScalar, 1.0, 1.0));
    
    // Color based on frame offset
    // float frameScalar = float(colorOffset % 1000) / 1000.0;
    // vec3 rgbColor = hsv2rgb(vec3(frameScalar, 1.0, 1.0));
    
    // White
    // vec4 positionColor = vec4(1.0,1.0,1.0,1.0);//vec4(rgbColor.rgb, 1.0);
    // vec4 positionColor = vec4(rgbColor.rgb, 1.0);
    
    int fftBand = int(64.0 * spectrumScalar) % 64;
    
    float volume = fft[fftBand];
    
    // Boost volume at the end of the spectrum
    float scalarSpectrum = float(fftBand) / 64.0;
    volume *= 1.0 + (scalarSpectrum * 3);
    volume *= 0.25;
    
    float fftHeightMulti = 1.0;
    vec4 pickedColor = vec4(1,1,1,1);
    
    float alpha = volume;
    float brightness = alpha;
    vs_out.color =  vec4(pickedColor.r * brightness,
                         pickedColor.g * brightness,
                         pickedColor.b * brightness,
                         alpha * alphaMultiplier);
    
    // Height and position
    float textureHeight = texture( heightMap, vec2(position.x, position.z) ).x;
    
    if (mountainMultiplier > 0)
    {
        // Get the distance from the tank
        vec3 groundPosition = (groundOffset + position) * groundScale;
        vec3 antiVector = vec3(1.0,1.0,1.0) - tankVector;
        float unitDistanceFromTank = length(((groundPosition - tankPosition) / groundScale) * antiVector);
        // non-linear
        unitDistanceFromTank *= unitDistanceFromTank;
        textureHeight *= 1.0 + (unitDistanceFromTank * mountainMultiplier);
    }
    
    gl_Position = projection * modelView * vec4(position.x,
                                                position.y + textureHeight,
                                                position.z,
                                                1.0);

    
    vs_out.fog = gl_Position.z;
}
