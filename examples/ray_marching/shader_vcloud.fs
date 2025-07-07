#version 330 core
out vec4 FragColor;

uniform float fovy; // in radians
uniform float width_ratio;
uniform vec2  resolution;

uniform vec3 camera_pos;
uniform vec3 camera_dir;
uniform mat4 world2view; // lookat matrix

uniform vec3 aabb_min;
uniform vec3 aabb_max;

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

void main() {
    vec2 uv = (gl_FragCoord.xy / resolution.xy) * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    float focal_length = 1.0 / tan(fovy * 0.5);

    vec3 rd_view = normalize(vec3(uv, -focal_length));
    vec3 rd      = normalize((inverse(world2view) * vec4(rd_view, 0.0)).xyz);
    vec3 ro      = camera_pos;

    // Slab method for ray-AABB intersection
    vec3 t0   = (aabb_min - ro) / rd;
    vec3 t1   = (aabb_max - ro) / rd;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float t_enter = max(max(tmin.x, tmin.y), tmin.z);
    float t_exit  = min(min(tmax.x, tmax.y), tmax.z);
    float t_step  = (t_exit - t_enter) / (nb_iter - 1);

    vec3  color         = vec3(0.); //?
    float sigma_t       = sigma_a + sigma_s;
    float transmittance = 1.;

    for (int it = 0; it < nb_iter; it++) {
        vec3 sample_pos = ro + rd * (t_enter + t_step * it);
        vec3 cloud_uvw  = (cloud_world2tex * vec4(sample_pos, 1)).xyz;
        vec3 light_uvw  = (light_cache_world2tex * vec4(sample_pos, 1)).xyz;

        float density   = texture(cloud_tex, cloud_uvw).x;
        float luminance = texture(light_tex, light_uvw).x;

        float T = exp(-density * t_step * sigma_t);

        transmittance *= T;
        if (transmittance < 0.01) break;

        color += transmittance * light_color * luminance * sigma_s / sigma_t;
    }
    color += transmittance * bkgd_color;

    FragColor = vec4(color, 1.);
}