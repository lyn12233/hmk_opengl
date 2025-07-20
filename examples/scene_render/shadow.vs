#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 light_world2clip; // light projection * view

void main() { gl_Position = light_world2clip * vec4(aPos, 1.0); }
