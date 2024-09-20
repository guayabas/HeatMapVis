#version 330 core
layout (location = 0) in vec4 inputVertex;
out vec2 vTextureCoordinates;
void main()
{
    gl_Position = vec4(inputVertex.xy, 0.0, 1.0);
    vTextureCoordinates = inputVertex.zw;
}