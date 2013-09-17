varying vec4 colorVarying;

void main()
{
    float whiteness = gl_Vertex.y * -0.005;
    colorVarying = vec4(whiteness * 0.5,0.0,whiteness*0.75,1.0);
    gl_Position = ftransform();
}

