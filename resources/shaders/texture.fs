#version 330 core
in vec2 vTextureCoordinates;
uniform sampler2D uTextureSampler;
uniform float uScalarFieldMax;
uniform float uScalarFieldMin;
out vec4 fragColor;
vec3 getColorForValue(float value) 
{
    vec3 coldColor = vec3(0.0, 0.0, 0.0);
    vec3 hotColor = vec3(1.0, 1.0, 1.0);
    float normalized = (value - uScalarFieldMin) / (uScalarFieldMax - uScalarFieldMin);
    return mix(coldColor, hotColor, normalized);
}
void main()
{
    float colorFromTexture = texture(uTextureSampler, vTextureCoordinates).r;
    fragColor = vec4(getColorForValue(colorFromTexture), 1.0);
}