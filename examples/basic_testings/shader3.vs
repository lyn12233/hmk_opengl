#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 tex_coord;

out VS_OUT{ 
    vec4 current_color;
    vec3 fragPos;
    vec2 tex_coord;
} vs_out;


uniform mat4 model2world;//model
uniform mat4 world2view;//view

void main(){
    gl_Position = model2world*vec4(aPos,1.);
    vs_out.current_color=vec4(1-aPos.x,aPos.y,aPos.z,1.);
    vs_out.fragPos = (model2world * vec4(aPos,1)).xyz;
    vs_out.tex_coord = tex_coord;
}