#version 330 core

in GS_OUT {
    vec4 current_color;
    vec3 normal;
    vec2 tex_coord;
    vec3 fragPos;
}
gs_out;

out vec4 FragColor;

struct light_t {
    vec3 pos;
    vec3 diffuse;
    vec3 specular;
};
struct material_t {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

uniform vec3       viewPos;
uniform vec3       ambient;
uniform light_t    light;
uniform material_t material;

uniform mat4 world2view;

void main() {
    // ambient
    vec3 the_ambient = ambient * texture(material.diffuse, gs_out.tex_coord).rgb;

    // diffuse
    vec3  norm     = normalize(-gs_out.normal);
    vec3  lightDir = normalize(light.pos - gs_out.fragPos);
    float factor   = max(dot(norm, lightDir), 0);
    vec3  diffuse  = light.diffuse * factor * texture(material.diffuse, gs_out.tex_coord).rgb;

    // specular
    vec3 viewDir  = normalize(viewPos - gs_out.fragPos);
    vec3 halfway  = normalize(lightDir + viewDir);
    factor        = max(dot(norm, halfway), 0);
    vec3 specular = light.specular * pow(factor, material.shininess) *
                    texture(material.specular, gs_out.tex_coord).rgb;

    FragColor = vec4(the_ambient + diffuse + specular, 1);
}