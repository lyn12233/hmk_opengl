#version 330 core
out vec4 FragColor;

uniform float fovy;       // in radians, default pi/4
uniform vec2  resolution; // default (800,600)

uniform vec3 camera_pos; // default (0,0,50)
uniform mat4 world2view; // lookat matrix

uniform vec3 aabb_min; // default (-5,-5,-5)
uniform vec3 aabb_max; // default (5,5,5)

uniform mat4      cloud_world2tex;
uniform sampler3D perlin_tex1; // 32-bit float, 1 channel
uniform sampler3D perlin_tex2; // 32-bit float, 1 channel
uniform sampler3D perlin_tex3; // 32-bit float, 1 channel
uniform sampler3D perlin_tex4; // 32-bit float, 1 channel
uniform float     amp1;
uniform float     amp2;
uniform float     amp3;
uniform float     amp4;
uniform float     density_offs;
uniform float     density_max;

uniform int   nb_iter;
uniform int   nb_iter2;
uniform float max_length;

uniform vec3  bkgd_color;
uniform vec3  light_color;
uniform float sigma_a;
uniform float sigma_s;
uniform vec3  light_dir;
uniform vec4  phase_parm; // g1,g2,offs,scale

vec3 up = vec3(0, 1, 0);
#define SMALL_VALUE (1e-6)
#define MED_VALUE (1e-4)

float HenyeyGreenstein(float cos_theta, float g) {
    float gg = g * g;
    return (1 - gg) / pow(1 + gg - 2 * g * cos_theta, 1.5) / (4. * radians(180));
}
float phase_func(float cos_theta) {
    float hg = HenyeyGreenstein(cos_theta, phase_parm.x) * 0.5 +
               HenyeyGreenstein(cos_theta, phase_parm.y) * 0.5;
    return phase_parm.z + hg * phase_parm.w;
}
bool within_aabb(vec3 sample_pos) {
    return all(greaterThanEqual(sample_pos - aabb_min, vec3(1e-6))) &&
           all(greaterThanEqual(aabb_max - sample_pos, vec3(1e-6)));
}
float cloud_sampler(vec3 sample_pos) {
    if (!within_aabb(sample_pos)) {
        return 0.;
    }

    vec3  cloud_uvw = (cloud_world2tex * vec4(sample_pos, 1)).xyz;
    float n1        = texture(perlin_tex1, cloud_uvw).r;
    float n2        = texture(perlin_tex2, cloud_uvw).r;
    float n3        = texture(perlin_tex3, cloud_uvw).r;
    float n4        = texture(perlin_tex4, cloud_uvw).r;

    float density = n1 * amp1 + n2 * amp2 + n3 * amp3 + n4 * amp4;
    density       = max(density + density_offs, 0);

    if (density > density_max * 0.8) {
        density = (0.8 + 0.2 * (1 - exp((0.8 * density_max - density) / 0.2 / density_max))) *
                  density_max;
        // density = 0.8 * density_max;
    }

    return density;
}

float query_transmittance(vec3 ro, vec3 rd) {
    float transmittance = 1.;
    float t_step        = max_length / nb_iter2;

    for (int i = 0; i < max_length; i++) {
        vec3 sample_pos = ro + rd * (t_step * i);
        if (!within_aabb(sample_pos)) break;

        float density = cloud_sampler(ro + rd * (t_step * i));

        transmittance *= exp(-density * (sigma_a + sigma_s) * t_step);
        if (transmittance < 1e-4) break;
    }
    return transmittance;
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

    float t_enter = max(max(tmin.x, tmin.y), tmin.z) + 1e-4;
    float t_exit  = min(min(tmax.x, tmax.y), tmax.z) - 1e-4;
    float t_step  = (t_exit - t_enter) / (nb_iter - 1);

    t_enter = max(t_enter, 0);

    // this always render light_color
    if (t_exit < t_enter) {
        FragColor = vec4(bkgd_color, 1);
        return;
    }

    vec3  color         = vec3(0.); //?
    float radiance      = 0.;
    float sigma_t       = sigma_a + sigma_s;
    float transmittance = 1.;

    for (int it = 0; it < nb_iter; it++) {
        vec3 sample_pos = ro + rd * (t_enter + t_step * it);

        float density = cloud_sampler(sample_pos);
        if (density < 0) continue;

        float luminance = query_transmittance(sample_pos, -light_dir);

        // L_s = t_sun*(sigma)*P(cos)
        float light_s = luminance * density * (sigma_s / sigma_t) * phase_func(dot(-rd, light_dir));

        // dL_o = T L_s dt
        radiance += transmittance * light_s * t_step;

        float T = exp(-density * t_step * sigma_t);

        transmittance *= T;
    }
    // color += transmittance * bkgd_color;
    color = min(radiance, 1) * light_color + transmittance * bkgd_color;

    // correction: cloud+bkgd+sun_intensity+gamma

    FragColor = vec4(color, 1.);
}