#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in float aTexX;

out vec2 vTexCoord;

uniform float uWallHeight;

void main()
{
    float v = (1.0 - (aPos.y + 1.0) / 2.0) * uWallHeight;
    vTexCoord = vec2(aTexX, v);
    gl_Position = vec4(aPos, 0.0, 1.0);
}