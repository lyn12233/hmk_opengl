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
uniform float shininess;

// shadow mapping attr
uniform sampler2D shadow_tex;
uniform mat4      light_world2clip;

// uniform struct scales_t {
//     float ambient;
//     float diffuse;
//     float specular;
// } scales;

// utils
bool should_discard(vec2 uv) { return texture(gbuffer.t_pos, uv).z == 0.; }

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

    // if there is nothing, return
    if (should_discard(uv)) {
        discard;
    }

    // Lighting vectors
    vec3 N = normalize(norm);
    vec3 L = normalize(light_pos - pos);
    vec3 V = normalize(view_pos - pos);
    vec3 H = normalize(L + V);

    // Diffuse term (Lambert)
    float NdotL   = max(dot(N, L), 0.0);
    vec3  diffuse = c_diff * light_color * NdotL;

    // Specular term (Blinn‑Phong)
    float NdotH    = max(dot(N, H), 0.0);
    vec3  specular = c_spec * light_color * pow(NdotH, shininess);

    // Simple ambient
    vec3 ambient = c_diff;

    // shadow mapping
    vec4 light_clip;
    vec3 light_uv;

    light_clip = light_world2clip * vec4(pos, 1.0);
    light_clip.xyz /= light_clip.w;

    light_uv.xy = light_clip.xy * 0.5 + 0.5;
    light_uv.z  = light_clip.z * 0.5 + 0.5;

    float visibility, shadow_depth;

    if (light_uv.x < 0.0 || light_uv.x > 1.0 || light_uv.y < 0.0 || light_uv.y > 1.0) {
        visibility = 0.;
    } else {
        shadow_depth = texture(shadow_tex, light_uv.xy).r;
        visibility   = light_uv.z < shadow_depth + 1e-3 ? 1.0 : 0;
    }

    vec3 color;
    // color = ambient * scales.ambient +
    //         visibility * (diffuse * scales.diffuse + specular * scales.specular);
    color = ambient * s_ambient + visibility * (diffuse * s_diffuse + specular * s_specular);

    color     = color / (color + vec3(1));
    FragColor = vec4(color, 1.0);

    // test
    // FragColor = vec4(s_ambient, s_diffuse, s_specular, 1);
    // FragColor = vec4(vec3(visibility), 1);//some not whithin shadow map
    // FragColor = vec4(abs(pos), 1);
    // FragColor = vec4(normalize(norm - vec3(0, 1, 0)), 1);
}