#version 330 core
// this gshader is to set normals

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;


in VS_OUT{ 
    vec4 current_color;
    vec3 fragPos;
    vec2 tex_coord;
} vs_out[];

out GS_OUT{ 
    vec4 current_color;
    vec3 normal;
    vec2 tex_coord;
    vec3 fragPos;
} gs_out;

uniform mat4 world2view;//view
uniform mat4 view2clip;

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}  

void main(){
    gs_out.normal = GetNormal();
    gs_out.current_color = vs_out[0].current_color;
    gs_out.fragPos = vs_out[0].fragPos;
    gs_out.tex_coord = vs_out[0].tex_coord;
    gl_Position = view2clip * world2view * gl_in[0].gl_Position;
    EmitVertex();

    gs_out.normal = GetNormal();
    gs_out.current_color = vs_out[1].current_color;
    gs_out.fragPos = vs_out[1].fragPos;
    gs_out.tex_coord = vs_out[1].tex_coord;
    gl_Position = view2clip * world2view * gl_in[1].gl_Position;
    EmitVertex();

    gs_out.normal = GetNormal();
    gs_out.current_color = vs_out[2].current_color;
    gs_out.fragPos = vs_out[1].fragPos;
    gs_out.tex_coord = vs_out[2].tex_coord;
    gl_Position = view2clip * world2view * gl_in[2].gl_Position;
    EmitVertex();
    
    EndPrimitive();
}
