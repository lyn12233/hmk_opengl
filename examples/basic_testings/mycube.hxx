#pragma once

#include "buffer_objects.hxx"
#include "checkfail.hxx"
#include "drawable_frame.hxx"
#include "utils.hxx"
#include "window.hxx"
#include "world_view.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"

#include <memory>
#include <vector>

#include <spdlog/spdlog.h>
#include <glm/geometric.hpp>

#ifdef _DEBUG
static struct SpdlogInit {
    SpdlogInit() {
        spdlog::set_level(spdlog::level::debug);
    }
} spdlog_init;
#endif

using glm::vec3;
using glm::mat4;
using std::shared_ptr;

inline mat4 model2world(vec3 model_pos,vec3 model_tangent, vec3 model_bitangent){
    vec3 normal = glm::normalize(glm::cross(model_tangent, model_bitangent));
    model_bitangent = glm::normalize(glm::cross(normal,model_tangent));

    mat4 transform(1.0f);
    transform[0] = glm::vec4(glm::normalize(model_tangent), 0.0f);
    transform[1] = glm::vec4(glm::normalize(model_bitangent), 0.0f);
    transform[2] = glm::vec4(normal, 0.0f);
    transform[3] = glm::vec4(model_pos, 1.0f);
    return transform;
}

class MyCube{
    public:
    MyCube(
        vec3 tangent=vec3(1,0,0), 
        vec3 bitangent=vec3(0,1,0),
        vec3 pos=vec3(0,0,0)
    );

    std::vector<float> vertices;
    std::vector<int> indices;
    float scale;
    
    VertexArrayObject vao;
    VertexBufferObject vbo;
    BufferObject ebo;
    TextureObject diffuse;
    TextureObject specular;

    vec3 pos_;
    vec3 tangent_;
    vec3 bitangent_;
    // vec3 vec_z;
    inline mat4 model2world(){
        return ::model2world(pos_,tangent_,bitangent_);
    }

};
class MyWorld:public mf::WorldViewBase{
    public:
    MyWorld();
    bool draw(mf::DrawableFrame& fbo)override;

    std::vector<shared_ptr<MyCube>> cubes;
    shared_ptr<ShaderProgram> prog;

};