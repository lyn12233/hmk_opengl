#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform vec3 view_pos;

uniform struct {
    sampler2D t_pos;
    sampler2D t_norm;
    sampler2D t_diff;
    sampler2D t_spec;
} gbuffer;

// light attr
uniform vec3  light_pos;
uniform vec3  light_color;
uniform float shininess;

// shadow mapping attr
uniform sampler2D depth_tex;
uniform mat4      light_world2clip;

uniform float cursor;

// void main(){FragColor = vec4(texture(gbuffer.t_pos, gl_FragCoord.xy).xyz,1.);}

void main() {

    // vec2 uv = gl_FragCoord.xy / resolution;
    vec2 uv = texCoord;

    vec3 pos    = texture(gbuffer.t_pos, uv).xyz;
    vec3 norm   = texture(gbuffer.t_norm, uv).xyz;
    vec3 c_diff = texture(gbuffer.t_diff, uv).xyz;
    vec3 c_spec = texture(gbuffer.t_spec, uv).xyz;

    // if there is nothing, return
    if (pos.z == 0.) {
        discard;
    }

    // Lighting vectors
    vec3 N = norm;
    vec3 L = normalize(light_pos);
    vec3 V = normalize(view_pos - pos);
    vec3 H = normalize(L + V);

    // Diffuse term (Lambert)
    float NdotL   = max(dot(N, L), 0.0);
    vec3  diffuse = c_diff * light_color * NdotL;

    // Specular term (Blinn‑Phong)
    float NdotH    = max(dot(N, H), 0.0);
    vec3  specular = c_spec * light_color * pow(NdotH, shininess);

    // Simple ambient
    vec3 ambient = 0.1 * c_diff;

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
        vec3 tex     = texture(depth_tex, light_uv.xy).rgb;
        shadow_depth = max(max(tex.r, tex.g), tex.b);
        visibility   = light_uv.z < shadow_depth + 2e-7 ? 1.0 : 0;
    }

    vec3 color = ambient + visibility * (diffuse + specular);
    FragColor  = vec4(color, 1.0);
    // FragColor = vec4(vec3(light_uv.z - 1 + cursor / 100 > 0), 1);
    // FragColor = vec4(vec3(shadow_depth - 1 + cursor / 100 > 0), 1);
    // FragColor = vec4(vec3(light_uv.z <= shadow_depth + 2e-7), 1);
    // FragColor = vec4(pos / 40., 1);
    // FragColor = vec4(norm, 1);
    // FragColor = vec4(c_diff, 1);
    // FragColor = vec4(c_spec, 1);
    // FragColor = vec4(vec3(visibility), 1.);
}