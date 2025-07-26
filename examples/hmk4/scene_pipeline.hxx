#pragma once

#include "buffer_objects.hxx"
#include "drawable_frame.hxx"
#include "parameter_dict.hxx"
#include "shader_program.hxx"
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

    void render_scene_defr(                                                   //
        const mf::DrawableFrame                &fbo,                          //
        mf::Rect                                cur_rect,                     //
        std::vector<std::shared_ptr<ModelBase>> models,                       //
        const FrameBufferObject                &gbuffer,                      //
        const FrameBufferObject                &shadow_buffer,                //
        mat4 shadow_mapping, mat4 world2clip, mat4 world2view, vec3 view_pos, //
        mf::ParameterDict &arguments
    );

    static std::shared_ptr<ShaderProgram>      prog_shade;
    static std::shared_ptr<ShaderProgram>      prog_draw;
    static std::shared_ptr<VertexArrayObject>  vao;
    static std::shared_ptr<VertexBufferObject> vbo;
} // namespace hmk4_models