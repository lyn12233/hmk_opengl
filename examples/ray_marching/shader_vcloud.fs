#version 330 core
out vec4 FragColor;

uniform float fovy;       // in radians, default pi/4
uniform vec2  resolution; // default (800,600)

uniform vec3 camera_pos; // default (0,0,50)
uniform mat4 world2view; // lookat matrix

uniform vec3 aabb_min; // default (-5,-5,-5)
uniform vec3 aabb_max; // default (5,5,5)

uniform mat4      cloud_world2tex;
uniform mat4      light_cache_world2tex;
uniform sampler3D cloud_tex; // 32-bit float, 1 channel
uniform sampler3D light_tex; // 32-bit float, 1 channel
uniform int       nb_iter;

uniform vec3  bkgd_color;
uniform vec3  light_color;
uniform float sigma_a;
uniform float sigma_s;

vec3 up = vec3(0, 1, 0);

bool is_within_aabb(in vec3 aabb_max, in vec3 aabb_min, in vec3 pos) {
    vec3 aabb_center    = (aabb_max + aabb_min) / 2;
    vec3 aabb_size_half = (aabb_max - aabb_min) / 2;
    vec3 rel_dist       = abs((pos - aabb_center) / aabb_size_half);

    float sdf = max(max(rel_dist.x, rel_dist.y), rel_dist.z);
    return sdf < 1.;
}

void main() {
    vec2 uv = (gl_FragCoord.xy / resolution.xy) * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    float focal_length = 1.0 / tan(fovy * 0.5);

    vec3 rd_view  = normalize(vec3(uv, -focal_length));
    vec3 ray_dest = (inverse(world2view) * vec4(rd_view, 1)).xyz;
    vec3 rd       = normalize(ray_dest - camera_pos);
    vec3 ro       = camera_pos;

    // Slab method for ray-AABB intersection
    vec3 safe_rd = rd;
    if (abs(safe_rd.x) < 1e-6) safe_rd.x = 1e-6;
    if (abs(safe_rd.y) < 1e-6) safe_rd.y = 1e-6;
    if (abs(safe_rd.z) < 1e-6) safe_rd.z = 1e-6;
    vec3 t0   = (aabb_min - ro) / safe_rd;
    vec3 t1   = (aabb_max - ro) / safe_rd;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float t_enter = max(max(tmin.x, tmin.y), tmin.z);
    float t_exit  = min(min(tmax.x, tmax.y), tmax.z);
    float t_step  = (t_exit - t_enter) / (nb_iter - 1);

    // this always render light_color
    if (t_exit < max(t_enter, 0)) {
        FragColor = vec4(bkgd_color, 1);
        return;
    }

    vec3  color         = vec3(0.); //?
    float sigma_t       = sigma_a + sigma_s;
    float transmittance = 1.;

    for (int it = 0; it < nb_iter; it++) {
        vec3 sample_pos = ro + rd * (t_enter + t_step * it);
        vec3 cloud_uvw  = (cloud_world2tex * vec4(sample_pos, 1)).xyz;
        vec3 light_uvw  = (light_cache_world2tex * vec4(sample_pos, 1)).xyz;

        bool in_cloud_tex =
            all(greaterThanEqual(cloud_uvw, vec3(0))) && all(lessThanEqual(cloud_uvw, vec3(1)));
        bool in_light_tex =
            all(greaterThanEqual(light_uvw, vec3(0))) && all(lessThanEqual(light_uvw, vec3(1)));

        float density   = in_cloud_tex ? texture(cloud_tex, cloud_uvw).x : 0.01;
        float luminance = in_light_tex ? texture(light_tex, light_uvw).x : 0.01;

        float T = exp(-density * t_step * sigma_t);

        transmittance *= T;
        if (transmittance < 0.01) break;

        // color += transmittance * light_color * luminance * sigma_s / sigma_t;
        color += light_color * 0.;
    }
    color += transmittance * bkgd_color;

    FragColor = vec4(color, 1.);
}