#version 330 core
layout (location = 0) in vec2 inputVertex;
void main()
{
    gl_Position = vec4(inputVertex, 0.0, 1.0);
}