#version 330 core
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uShade;
out vec4 FragColor;
void main()
{
    vec4 texColor = texture(uTexture, vTexCoord);
    FragColor = texColor * uShade;
}