#version 330 core
in vec2  texCoord;
out vec4 FragColor;

uniform sampler2D depth_tex;

void main() {
    float d = texture(depth_tex, texCoord).r; // depth component

    d = d == 1. ? 1 : 0;

    FragColor = vec4(vec3(d), 1.0); // visualize as grayscale
}
