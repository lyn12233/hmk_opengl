#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform vec3 view_pos;

uniform struct {
    sampler2D t_pos;
    sampler2D t_norm;
    sampler2D t_diff;
    sampler2D t_spec;
    sampler2D t_vis;
} gbuffer;

// light attr
uniform vec3  light_pos;
uniform vec3  light_color;
uniform float s_light;
uniform float shininess;

// cloud
uniform struct CLOUD_T_ {
    vec3      aabb_min;
    vec3      aabb_max;
    float     sigma_a;
    float     sigma_s;
    mat4      world2tex;
    sampler3D tex;
} cloud;
uniform mat4  world2view;
uniform float fovy;

const float s_emit            = 0.05;
const float nb_iter1          = 30;
const float nb_iter2          = 10;
const float strength_modifier = 8.;
const vec3  bkgd_color        = vec3(0.53, 0.81, .92);
const float s_bkgd            = .5;

// utils
bool should_discard(vec2 uv) { return texture(gbuffer.t_pos, uv).z == 0.; }

// ray marching
void draw_sky(vec2 uv, vec3 bkdg_color, vec3 block_pos, bool test_block);

void main() {

    vec2 uv = texCoord;

    vec3  pos    = texture(gbuffer.t_pos, uv).xyz;
    vec3  norm   = texture(gbuffer.t_norm, uv).xyz;
    vec3  c_diff = texture(gbuffer.t_diff, uv).xyz;
    vec3  c_spec = texture(gbuffer.t_spec, uv).xyz;
    float vis    = texture(gbuffer.t_vis, uv).r;

    float s_ambient  = texture(gbuffer.t_pos, uv).w;
    float s_diffuse  = texture(gbuffer.t_diff, uv).w;
    float s_specular = texture(gbuffer.t_spec, uv).w;

    // if there is nothing, set sky color
    vec3 color;
    bool test_block;
    if (should_discard(uv)) {
        // draw_sky(uv);
        // return;
        color      = bkgd_color / (vec3(1) - bkgd_color) * s_bkgd;
        test_block = false;
    } else {

        // Lighting vectors
        vec3 N = normalize(norm);
        vec3 L = normalize(light_pos - pos);
        vec3 V = normalize(view_pos - pos);
        vec3 H = normalize(L + V);

        // Diffuse term (Lambert)
        float NdotL   = max(dot(N, L), 0.0);
        vec3  diffuse = c_diff * NdotL;

        // Specular term (Blinn‑Phong)
        float NdotH    = max(dot(N, H), 0.0);
        vec3  specular = c_spec * pow(NdotH, shininess);

        // Simple ambient
        vec3 ambient = c_diff;

        color = ambient * s_ambient + vis * (diffuse * s_diffuse + specular * s_specular);
        color *= light_color.xyz * s_light;
        test_block = true;
    }
    draw_sky(uv, color, pos, test_block);

    // test
    // FragColor = vec4(vec3(vis), 1);
    // FragColor = vec4(s_ambient, s_diffuse, s_specular, 1);
    // FragColor = vec4(vec3(visibility), 1);//some not whithin shadow map
    // FragColor = vec4(abs(pos), 1);
    // FragColor = vec4(normalize(norm - vec3(0, 1, 0)), 1);
}

// phase function parts
float HG(float cos_theta, float g) {
    float gg = g * g;
    return (1 - gg) / pow(1 + gg - 2 * g * cos_theta, 1.5) / (4. * radians(180));
}

float phase_func(float cos_theta) {
    float hg = HG(cos_theta, 0.83) * 0.5 + HG(cos_theta, 0.3) * 0.5;
    return 0.3 + hg * 2.;
}

// reuires: struct cloud
void query_intersect_range(vec3 ro, vec3 rd, out float t_enter, out float t_exit) {
    vec3 safe_rd = rd;
    if (abs(safe_rd.x) < 1e-6) safe_rd.x = 1e-6;
    if (abs(safe_rd.y) < 1e-6) safe_rd.y = 1e-6;
    if (abs(safe_rd.z) < 1e-6) safe_rd.z = 1e-6;
    vec3 t0   = (cloud.aabb_min - ro) / safe_rd;
    vec3 t1   = (cloud.aabb_max - ro) / safe_rd;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    t_enter   = max(max(max(tmin.x, tmin.y), tmin.z) + 1e-4, 0);
    t_exit    = min(min(tmax.x, tmax.y), tmax.z) - 1e-4;
}

// requires: struct cloud
float sample_cloud(vec3 pos) {
    vec3  cloud_uvw = (cloud.world2tex * vec4(pos, 1)).xyz;
    float density   = texture(cloud.tex, cloud_uvw).r;

    return abs(density);
}

// query transmittance to light
float query_transmittance(vec3 pos) {
    vec3 ro = pos;
    vec3 rd = normalize(light_pos - pos);

    float t_enter, t_exit, t_step;
    query_intersect_range(ro, rd, t_enter, t_exit);
    t_step = max((t_exit - t_enter) / (nb_iter2 - 1), 0.1);
    if (t_exit < t_enter) return 1.;

    float dist = 0;
    float cur_step, density;
    for (float t = t_enter; t <= t_exit; t += cur_step) {
        density  = sample_cloud(ro + rd * t);
        cur_step = density < 1e-2 ? t_step * 2 : t_step;
        dist += cur_step * density;
    }
    float sigma_t = cloud.sigma_a + cloud.sigma_s;
    return exp(-sigma_t * dist) * (1 - exp(-sigma_t * dist * 2));
}

// vec3 HDR(vec3 color){
//     color=color/(color+vec3(1));
// }

// requires: fovy, world2view, view_pos
void draw_sky(vec2 uv, vec3 cur_bkdg, vec3 block_pos, bool test_block) {
    float sigma_s = cloud.sigma_s;
    float sigma_a = cloud.sigma_a;
    float sigma_t = cloud.sigma_a + cloud.sigma_s;

    float focal_length = 1 / tan(fovy * 0.5);

    vec3 rd = normalize( //
        (inverse(world2view) * vec4(uv * 2 - 1, -focal_length, 0)).xyz
    );
    vec3 ro = view_pos;

    // vec3 rdest = normalize(vec3(uv * 2 - 1, -focal_length));
    // rdest      = (inverse(world2view) * vec4(rdest, 1)).xyz;
    // rd         = normalize(rdest - view_pos);

    // FragColor = vec4(uv * 2 - 1, focal_length, 1);
    // FragColor = vec4(rd, 1);
    // FragColor = vec4(0, 1, 0, 0);
    // return;

    float t_enter, t_exit, t_step;
    query_intersect_range(ro, rd, t_enter, t_exit);
    t_step = max((t_exit - t_enter) / (nb_iter1 - 1), 0.1);
    // if (t_exit < t_enter) {
    //     FragColor = vec4(cur_bkdg, 1);
    //     // return;
    // }

    float radiance      = 0;
    float transmittance = 1.;
    float cur_step, density;
    for (float t = t_enter; t <= t_exit; t += cur_step) {
        vec3 sample_pos = ro + rd * t;
        if (length(sample_pos - view_pos) >= length(block_pos - view_pos) && test_block) break;

        // sample and adapt step
        density  = sample_cloud(sample_pos);
        cur_step = density < 1e-2 ? t_step * 2 : t_step;

        // calc light
        float luminance = query_transmittance(sample_pos) * s_light;
        // float luminance = s_light;

        float L_o = luminance * phase_func(dot(-rd, normalize(light_pos - sample_pos))) * sigma_s /
                        sigma_t + //
                    s_emit * sigma_a / sigma_t;

        // integral
        radiance += transmittance * L_o * density * cur_step;
        transmittance *= exp(-density * sigma_t * cur_step);

        if (transmittance < 5e-4) break;
    }

    vec3 color = (radiance * light_color * strength_modifier + transmittance * cur_bkdg);

    // hdr, gamma coorection,
    color = color / (color + vec3(1));
    color = pow(color, vec3(0.45));
    // color = mix(color, cur_bkdg, transmittance);
    // color = abs(color - cur_bkdg);

    FragColor = vec4(color, 1);
}
