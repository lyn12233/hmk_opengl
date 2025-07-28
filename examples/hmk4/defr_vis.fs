#version 330 core

layout(location = 0) out vec4 t_vis;

in vec2 texCoord;

uniform struct {
    sampler2D t_pos;
    sampler2D t_norm;
} gbuffer;

// shadow mapping attr
uniform sampler2D shadow_tex;
uniform mat4      world2shadow;

uniform mat4 world2clip;

uniform float cursor; //=2

// utils
vec3 query_pos(vec2 uv) { return texture(gbuffer.t_pos, uv).xyz; }
vec3 query_norm(vec2 uv) { return normalize(texture(gbuffer.t_norm, uv).xyz); }
bool should_discard(vec2 uv) { return texture(gbuffer.t_pos, uv).z == 0.; }

float depth2vis(float cur_depth, float tex_depth) { //
    return cur_depth < tex_depth + cursor ? 1. : (cur_depth - tex_depth - cursor) * 1e-5;
}

float query_visibility(vec3 pos) {
    // get pos' uv in gbuffer to adjust pos
    vec4 pos_clip = world2clip * vec4(pos, 1.0);
    vec2 pos_uv   = pos_clip.xy / pos_clip.w * 0.5 + 0.5;
    if (should_discard(pos_uv)) return 0.;
    vec3 pos2 = query_pos(pos_uv);
    // return 1;
    pos = pos2;

    // get light map's uv
    vec4 light_clip = world2shadow * vec4(pos, 1.0);
    vec3 light_uv;
    light_uv.xy = light_clip.xy / light_clip.w * 0.5 + 0.5;

    // check uv threshold
    float visibility;
    if (light_uv.x < 0.0 || light_uv.x > 1.0 || light_uv.y < 0.0 || light_uv.y > 1.0) {
        visibility = 0.;
    } else {
        float tex_depth = texture(shadow_tex, light_uv.xy).r;
        tex_depth       = (tex_depth * 2 - 1) * light_clip.w;
        visibility      = depth2vis(light_clip.z, tex_depth);
    }
    return visibility;
}

float query_vis_aa(vec3 pos, vec3 norm, float dist) {
    vec3 va = dot(vec3(1, 1, 0), norm) < 1e-5 ? vec3(1, 1, 0) : vec3(0, 1, 1);
    vec3 vb = normalize(cross(norm, va));
    va      = normalize(cross(vb, norm));
    vec3 points[9];
    points[0] = pos;
    points[1] = pos + va * dist;
    points[2] = pos - va * dist;
    points[3] = pos + vb * dist;
    points[4] = pos - vb * dist;
    points[5] = pos + normalize(va + vb) * dist;
    points[6] = pos + normalize(va - vb) * dist;
    points[7] = pos - normalize(va + vb) * dist;
    points[8] = pos - normalize(va - vb) * dist;
    float portions[9];
    portions[0] = 1., portions[1] = 1., portions[2] = 1., portions[3] = 1., portions[4] = 1.;
    portions[5] = 1., portions[6] = 1., portions[7] = 1., portions[8] = 1.;
    float portion    = 0;
    float visibility = 0;
    for (int i = 0; i < 9; i++) {
        float vis = query_visibility(points[i]);
        if (vis >= 0.) {
            portion += portions[i];
            visibility += vis;
        }
    }
    if (portion == 0.) return 0.;
    return visibility / portion;
}

void main() {
    float sample_dist = cursor * 1e-5; // unused
    sample_dist       = .1;

    vec2 uv = texCoord;

    vec3 pos  = texture(gbuffer.t_pos, uv).xyz;
    vec3 norm = texture(gbuffer.t_norm, uv).xyz;
    norm      = normalize(norm);

    if (should_discard(uv)) {
        discard;
    }

    t_vis.r = query_vis_aa(pos, norm, sample_dist);
}
