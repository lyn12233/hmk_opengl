#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 4) in vec2 aTex;
layout(location = 5) in vec4 aColor;

out vec3 pos;
out vec3 norm;
out vec4 color;
out vec2 tex_coord;

uniform mat4 model2clip;
uniform mat4 model2world;

void main() {
    pos         = (model2world * vec4(aPos, 1)).xyz;
    norm        = (model2world * vec4(aNorm, 0)).xyz;
    color       = aColor;
    tex_coord   = aTex;
    gl_Position = model2clip * vec4(aPos, 1);
}