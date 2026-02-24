#version 330 core
in vec2 vUV;
uniform sampler2D uScreen;
out vec4 FragColor;
void main() {
    FragColor = texture(uScreen, vUV);
}