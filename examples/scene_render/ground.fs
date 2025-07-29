#version 330 core
in vec2 tex_coord;
in vec3 pos;

uniform vec3 c1;
uniform vec3 c2; // unused

uniform vec3  light_color;
uniform vec3  light_pos;
uniform float shininess;

uniform vec3 view_pos;

uniform sampler2D tex1;
uniform float     hscale;

uniform float pix_per_m;

uniform float s_ambient;
uniform float s_diffuse;
uniform float s_specular;

out vec4 FragColor;

uniform mat4 world2tex;

const vec3 BaseN = vec3(0, 1, 0);

float sgm(float t) { return (exp(t) - exp(-t)) / (exp(t) + exp(-t)); }

float sample_height(vec2 uv) {
    vec2  uv1 = vec2(uv.x, 1 - uv.y);
    vec2  uv2 = vec2(1 - uv.x, 1 - uv.y);
    float w1  = abs(uv.x * 2 - 1);
    float w2  = abs(uv.y * 2 - 1);
    float h   = (texture(tex1, uv).r + texture(tex1, uv1).r * w1 + texture(tex1, uv2).r * w2) /
              (1. + w1 + w2);
    return sgm(h * hscale) * hscale;
}

vec2 remap_coord(vec2 coord, const vec3 projV, const vec3 pos) {
    float last_h = 0.;
    float h;
    vec3  pos2   = pos;
    vec2  coord2 = coord;
    for (int i = 0; i < 4; i++) {
        h      = sample_height(coord2);
        pos2   = pos2 + (h - last_h) * projV;
        coord2 = (world2tex * vec4(pos2, 1)).xy;
        last_h = h;
    }
    return coord2;
}

void main() {

    // prallax mapping

    vec3 V     = normalize(view_pos - pos);
    vec3 projV = V - dot(BaseN, V) * BaseN;

    vec2  tex_coord2 = remap_coord(tex_coord, projV, pos);
    float h          = sample_height(tex_coord2);
    vec3  c          = mix(c1, c2, exp(-1 - h));

    // calc normal
    float tex_dist = 1. / 1000. / pix_per_m;

    float du = (sample_height(tex_coord2 + vec2(tex_dist, 0)) -
                sample_height(tex_coord2 - vec2(tex_dist, 0))) /
               2 * pix_per_m;
    float dv = (sample_height(tex_coord2 + vec2(0, tex_dist)) -
                sample_height(tex_coord2 - vec2(0, tex_dist))) /
               2 * pix_per_m;

    vec3 L = normalize(light_pos - pos);
    vec3 N = normalize(vec3(-du, 1, -dv));
    vec3 H = normalize(L + V);

    // Diffuse term (Lambert)
    float NdotL   = max(dot(N, L), 0.0);
    vec3  diffuse = c * light_color * NdotL;

    // Specular term (Blinn‑Phong)
    float NdotH    = max(dot(N, H), 0.0);
    vec3  specular = c * light_color * pow(NdotH, shininess);

    vec3 color = c * s_ambient + diffuse * s_diffuse + specular * s_specular;
    color      = color / (color + vec3(1));
    FragColor  = vec4(color, 1);
}