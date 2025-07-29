#version 330 core

layout(location = 0) in vec3 aPos;

out vec2 tex_coord;
out vec3 pos;

uniform mat4 model2clip;
uniform mat4 world2tex;

void main() {
    pos         = aPos;
    tex_coord   = (world2tex * vec4(aPos, 1)).xy;
    gl_Position = model2clip * vec4(aPos, 1);
}