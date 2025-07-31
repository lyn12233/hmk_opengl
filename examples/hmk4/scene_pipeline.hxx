#pragma once

#include "buffer_objects.hxx"
#include "drawable_frame.hxx"
#include "parameter_dict.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
#include "utils.hxx"

#include <memory>
#include <vector>

namespace hmk4_models {
    using namespace glwrapper;
    class ModelBase {
        public:
        virtual ~ModelBase() = default;
        virtual void draw_gbuffer(glm::mat4 world2clip, glm::mat4 world2view);
        virtual void draw(
            std::vector<std::shared_ptr<ShaderProgram>> progs, glm::mat4 world2clip,
            glm::mat4 world2view, bool require_sampler
        );
        virtual void draw(
            std::shared_ptr<ShaderProgram> prog, glm::mat4 world2clip, glm::mat4 world2view,
            bool require_sampler
        );

        // data
        glm::mat4 model2world_;
    };
    class CloudModelBase {
        public:
        vec3  aabb_min_;
        vec3  aabb_max_;
        float sigma_a_, sigma_s_;
        mat4  world2tex_;

        std::shared_ptr<TextureObject> tex_;

        public:
        virtual ~CloudModelBase() = default;
        virtual void activate_cloud_sampler(std::shared_ptr<ShaderProgram> prog, int at);
    };

    void render_scene_defr(                                    //
        const mf::DrawableFrame                &fbo,           //
        mf::Rect                                cur_rect,      //
        std::vector<std::shared_ptr<ModelBase>> models,        //
        std::shared_ptr<CloudModelBase>         cloud,         //
        const FrameBufferObject                &gbuffer,       //
        const FrameBufferObject                &shadow_buffer, //
        mat4 shadow_mapping, mat4 world2clip, mat4 world2view, //
        float fovy, vec3 view_pos,                             //
        mf::ParameterDict &arguments
    );

    // prog_shade input: layout(0) pos, uniform world2clip
    static std::shared_ptr<ShaderProgram>      prog_shade;
    static std::shared_ptr<ShaderProgram>      prog_draw;
    static std::shared_ptr<ShaderProgram>      prog_vis;
    static std::shared_ptr<VertexArrayObject>  vao;
    static std::shared_ptr<VertexBufferObject> vbo;
} // namespace hmk4_models