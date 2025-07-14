#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 5) in vec4 aColor;

out vec3 pos;
out vec3 norm;
out vec4 color;
out vec2 tex_coord;

uniform mat4 world2clip;

void main() {
    pos         = aPos;
    norm        = aNorm;
    color       = aColor;
    gl_Position = world2clip * vec4(pos, 1);
}