#version 330 core
layout(location = 0) in vec2 aPos; //{-1, -1, 1, -1, -1, 1, 1, 1}

void main() { //
    gl_Position = vec4(aPos, 1, 1);
}