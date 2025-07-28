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

uniform float cursor;

float depth2vis(float cur_depth, float tex_depth) { //
    return cur_depth < tex_depth + cursor ? 1. : (cur_depth - tex_depth - cursor) * 1e-5;
}

float query_visibility(vec3 pos) {
    // get map's uv
    vec4 light_clip = world2shadow * vec4(pos, 1.0);
    // light_clip.xyz /= light_clip.w;
    vec3 light_uv;
    light_uv.xy = light_clip.xy / light_clip.w * 0.5 + 0.5;
    // light_uv.z  = light_clip.z;
    // light_uv.z  = light_clip.z * 0.5 + 0.5;
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

void main() {
    float sample_dist = cursor * 1e-5; // unused
    sample_dist       = 0.05;

    vec2 uv = texCoord;

    vec3 pos  = texture(gbuffer.t_pos, uv).xyz;
    vec3 norm = texture(gbuffer.t_norm, uv).xyz;
    norm      = normalize(norm);

    if (pos.z == 0.) {
        discard;
    }

    vec3 va = dot(vec3(1, 0, 0), norm) < 1e-5 ? vec3(0, 1, 0) : vec3(1, 0, 0);
    vec3 vb = normalize(cross(norm, va));
    va      = normalize(cross(vb, norm));

    t_vis.r =
        ( //
            query_visibility(pos) + query_visibility(pos + va * sample_dist) +
            query_visibility(pos - va * sample_dist) + query_visibility(pos + vb * sample_dist) +
            query_visibility(pos - vb * sample_dist)
        ) /
        5.;
    // t_vis.r = query_visibility(pos);
    // t_vis.rgb = vec3(1);
}
