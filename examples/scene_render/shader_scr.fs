#version 330 core

out vec4 FragColor;

uniform vec2 resolution;

uniform vec3 view_pos;

uniform struct {
    sampler2D t_pos;
    sampler2D t_norm;
    sampler2D t_diff;
    sampler2D t_spec;
} gbuffer;

uniform vec3  light_dir;
uniform vec3  light_color;
uniform float shininess;

// void main(){FragColor = vec4(texture(gbuffer.t_pos, gl_FragCoord.xy).xyz,1.);}

void main() {

    vec2 uv = gl_FragCoord.xy / resolution;

    vec3 pos    = texture(gbuffer.t_pos, uv).xyz;
    vec3 norm   = texture(gbuffer.t_norm, uv).xyz;
    vec3 c_diff = texture(gbuffer.t_diff, uv).xyz;
    vec3 c_spec = texture(gbuffer.t_spec, uv).xyz;

    // if there is nothing, return
    if (pos.z == 0.) {
        // discard;
    }

    // Lighting vectors
    vec3 N = norm;
    vec3 L = normalize(-light_dir);
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

    vec3 color = ambient + diffuse + specular;
    FragColor  = vec4(color, 1.0);
    // FragColor  = vec4(gl_FragCoord.x, gl_FragCoord.y, 0, 1);
}