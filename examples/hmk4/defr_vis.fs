#version 400 core

layout(location = 0) out vec4 t_vis;

in vec2 texCoord;

uniform struct {
    sampler2D t_pos;
    sampler2D t_norm;
} gbuffer;

// shadow mapping attr
uniform sampler2D shadow_tex[8];
uniform float     shadow_portions[8];
uniform mat4      world2shadow[8];
uniform int       nb_shadow_tex;
uniform vec3      light_pos;

uniform mat4 world2clip;

uniform float cursor; //=2, control shadow depth smoothness

// cloud
uniform struct CLOUD_T_ {
    vec3      aabb_min;
    vec3      aabb_max;
    float     sigma_a;
    float     sigma_s;
    mat4      world2tex;
    sampler3D tex;
} cloud;
const float nb_iter = 30;

// utils
vec3 query_pos(vec2 uv) { return texture(gbuffer.t_pos, uv).xyz; }
vec3 query_norm(vec2 uv) { return normalize(texture(gbuffer.t_norm, uv).xyz); }
bool should_discard(vec2 uv) { return texture(gbuffer.t_pos, uv).z == 0.; }

float depth2vis(float cur_depth, float tex_depth) { //
    return cur_depth < tex_depth + cursor ? 1.
                                          : clamp((tex_depth + cursor - cur_depth) * 0.1 + 1, 0, 1);
    // return float(cur_depth < tex_depth + cursor);
}

float query_visibility_indexed(vec3 pos, int idx) {

    // get pos' uv in gbuffer to adjust pos
    vec4 pos_clip = world2clip * vec4(pos, 1.0);
    vec2 pos_uv   = pos_clip.xy / pos_clip.w * 0.5 + 0.5;
    if (should_discard(pos_uv)) return -1.;

    // adjust pos onto gbuffer
    vec3 pos2 = query_pos(pos_uv);
    pos       = pos2;

    // get light map's uv
    vec4 light_clip = world2shadow[idx] * vec4(pos, 1.0);
    vec3 light_uv;
    light_uv.xy = light_clip.xy / light_clip.w * 0.5 + 0.5;

    // check uv threshold
    float visibility;
    if (light_uv.x < 0.0 || light_uv.x > 1.0 || light_uv.y < 0.0 || light_uv.y > 1.0) {
        visibility = -1.;
    } else {
        // get world space depth
        float tex_depth = texture(shadow_tex[idx], light_uv.xy).r;
        tex_depth       = (tex_depth * 2 - 1) * light_clip.w;
        // compare depth smoothed
        visibility = depth2vis(light_clip.z, tex_depth);
    }
    return visibility;
}

float query_visibility(vec3 pos) {
    float vis     = 0.;
    float portion = 0.;
    for (int i = 0; i < min(nb_shadow_tex, 8); i++) {
        float v = query_visibility_indexed(pos, i);
        if (v >= 0.) {
            vis += v * shadow_portions[i];
            portion += shadow_portions[i];
        }
    }
    return clamp(portion != 0. ? vis / portion : 1., 0, 1);
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
    portions[0] = 4., portions[1] = 2., portions[2] = 2., portions[3] = 2., portions[4] = 2.;
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
    if (portion == 0.) return 1.;
    return visibility / portion;
}
float sample_cloud(vec3 pos) {
    vec3  cloud_uvw = (cloud.world2tex * vec4(pos, 1)).xyz;
    float density   = texture(cloud.tex, cloud_uvw).r;

    return abs(density);
}

// requires: struct cloud, light_pos, nb_iter
float query_transmittance(vec3 pos) {
    vec3 ro = pos;
    vec3 rd = normalize(light_pos - pos);

    // Slab method for ray-AABB intersection
    vec3 safe_rd = rd;
    if (abs(safe_rd.x) < 1e-6) safe_rd.x = 1e-6;
    if (abs(safe_rd.y) < 1e-6) safe_rd.y = 1e-6;
    if (abs(safe_rd.z) < 1e-6) safe_rd.z = 1e-6;
    vec3  t0      = (cloud.aabb_min - ro) / safe_rd;
    vec3  t1      = (cloud.aabb_max - ro) / safe_rd;
    vec3  tmin    = min(t0, t1);
    vec3  tmax    = max(t0, t1);
    float t_enter = max(max(max(tmin.x, tmin.y), tmin.z) + 1e-4, 0);
    float t_exit  = min(min(tmax.x, tmax.y), tmax.z) - 1e-4;
    float t_step  = (t_exit - t_enter) / (nb_iter - 1);

    if (t_exit < t_enter) {
        return 1.;
    }
    vec3  sample_pos = ro + rd * t_enter;
    float dist       = 0;
    float cur_step   = t_step;
    for (int it = 0; it < nb_iter; it++) {
        float density = sample_cloud(sample_pos);
        cur_step      = density < 1e-2 ? t_step * 2 : t_step;
        // cur_step      = t_step;
        dist += cur_step * density;
        sample_pos += rd * cur_step;
    }

    // beer's law
    float sigma_t = cloud.sigma_a + cloud.sigma_s;
    return exp(-sigma_t * dist);
    // correct:
    // return sample_cloud(ro + rd * t_enter / 2 + rd * t_exit / 2);
}

void main() {
    float sample_dist = cursor * 1e-5; // unused
    sample_dist       = .05;

    vec2 uv = texCoord;

    vec3 pos  = texture(gbuffer.t_pos, uv).xyz;
    vec3 norm = texture(gbuffer.t_norm, uv).xyz;
    norm      = normalize(norm);

    if (should_discard(uv)) {
        t_vis.r = 1.;
        return;
    }

    t_vis.r = query_vis_aa(pos, norm, sample_dist) * query_transmittance(pos);
    // t_vis.r = query_transmittance(pos);
    // t_vis.r = query_vis_aa(pos, norm, sample_dist);
    // t_vis.r = query_visibility(pos);
    // t_vis.r = 1.;
}
