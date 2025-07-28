#version 330 core

layout(location = 0) out vec4 g_pos;
layout(location = 1) out vec4 g_norm;
layout(location = 2) out vec4 g_diff;
layout(location = 3) out vec4 g_spec;

in vec3 pos;
in vec3 norm;
in vec4 color;
in vec2 tex_coord;

out vec4 FragColor;

uniform vec3 light_pos;
uniform vec3 view_pos;

uniform struct {
    sampler2D diffuse;
    sampler2D specular;
} Cylinder_001;

void main() {
    g_pos  = vec4(pos, 0.1);
    g_norm = vec4(norm, 1);

    vec2 tex_coord_ = tex_coord;

    tex_coord_.y = 1 - tex_coord.y;

    vec4 diff_color = texture(Cylinder_001.diffuse, tex_coord_);
    vec4 spec_color = texture(Cylinder_001.specular, tex_coord_);

    g_diff = vec4(diff_color.xyz, 2.);
    g_spec = vec4(spec_color.xyz, .1);
}