#version 330 core

layout(location = 0) out vec4 g_pos;
layout(location = 1) out vec4 g_norm;
layout(location = 2) out vec4 g_diff;
layout(location = 3) out vec4 g_spec;

in vec2 tex_coord;
in vec3 pos;

const vec3  c1         = vec3(0.54, 0.36, 0.23);
const vec3  c2         = vec3(0.81, 0.54, 0.33);
const float s_ambient  = 0.01;
const float s_diffuse  = 2.;
const float s_specular = 0.01;
const vec3  BaseN      = vec3(0, 1, 0);

uniform sampler2D height_map;
uniform float     pix_per_m;
uniform mat4      world2tex;
uniform vec3      view_pos;

float sample_height(vec2 uv) {
    uv = mod(uv, 1);
    return texture(height_map, uv).r;
}

// parallax mapping
void remap_coord(
    const vec2 coord, const vec3 projV, const vec3 pos, out vec3 pos2, out vec2 coord2
) {
    float last_h = 0.;
    float h;
    pos2   = pos;
    coord2 = coord;
    for (int i = 0; i < 4; i++) {
        h      = sample_height(coord2);
        pos2   = pos2 + (h - last_h) * projV;
        coord2 = (world2tex * vec4(pos2, 1)).xy;
        last_h = h;
    }
}

void main() {
    vec3 V     = normalize(view_pos - pos);
    vec3 projV = V - dot(BaseN, V) * BaseN;

    vec3 pos2;
    vec2 tex_coord2;
    remap_coord(tex_coord, projV, pos, pos2, tex_coord2);
    float h = sample_height(tex_coord2);
    vec3  c = mix(c1, c2, exp(-1 - h));

    float tex_dist = 1. / 1000. / pix_per_m;

    float du = (sample_height(tex_coord2 + vec2(tex_dist, 0)) -
                sample_height(tex_coord2 - vec2(tex_dist, 0))) /
               2 * pix_per_m;
    float dv = (sample_height(tex_coord2 + vec2(0, tex_dist)) -
                sample_height(tex_coord2 - vec2(0, tex_dist))) /
               2 * pix_per_m;

    // g_pos  = vec4(pos2, s_ambient);
    g_norm = vec4(normalize(vec3(-du, 1, -dv)), 1);
    g_diff = vec4(c, s_diffuse);
    g_spec = vec4(c, s_specular);
    g_pos  = vec4(pos, 1);
}