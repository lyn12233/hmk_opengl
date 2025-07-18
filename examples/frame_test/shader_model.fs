#version 330 core

in vec3 pos;
in vec3 norm;
in vec4 color;
in vec2 tex_coord;

out vec4 FragColor;

uniform vec3 light_dir;
uniform vec3 view_pos;

uniform struct {
    sampler2D diffuse;
    sampler2D specular;
} Cylinder_001;

void main() {
    vec3  N    = normalize(norm);
    vec3  L    = normalize(-light_dir);
    float diff = max(dot(N, L), 0.0);

    vec3  V    = normalize(view_pos - pos);
    vec3  R    = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), 32.0);

    vec2 tex_coord_ = tex_coord;

    tex_coord_.y = 1 - tex_coord.y;

    vec4 diff_color = texture(Cylinder_001.diffuse, tex_coord_);
    vec4 spec_color = texture(Cylinder_001.specular, tex_coord_);

    FragColor = (0.1 + 0.6 * diff) * diff_color + 0.3 * spec * spec_color;
}