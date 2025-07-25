#version 330 core

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec2 aPos;

out vec2 tex_coord;
out vec3 pos;

uniform mat4 world2clip;
uniform mat4 world2tex;

void main() {
    pos         = vec3(aPos.x, 0, aPos.y);
    tex_coord   = (world2tex * vec4(pos, 1)).xy;
    gl_Position = world2clip * vec4(pos, 1);
}