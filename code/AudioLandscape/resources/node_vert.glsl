uniform vec4 uColor;
uniform mat4 uModelViewProjectionMatrix;

varying vec4 colorVarying;

void main()
{
    gl_Position = uModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
    colorVarying = uColor;
}

