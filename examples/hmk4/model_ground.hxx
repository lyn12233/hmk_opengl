#pragma once

#include "hmk4_config.hxx"

#include "buffer_objects.hxx"
#include "model.hxx"
#include "scene_pipeline.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"

#include <memory>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

namespace hmk4_models {
    using namespace glwrapper;
    // constexpr float pi = glm::pi<float>();
    class Ground : public ModelBase {
        public:
        VertexArrayObject  vao;
        VertexBufferObject vbo;
        int                nb_verts;

        std::shared_ptr<ShaderProgram> prog_defr_ground;
        std::shared_ptr<TextureObject> height_map;

        constexpr static float chunk_width  = 100;
        constexpr static float chunk_height = 100;
        constexpr static float pix_per_m    = 2;
        constexpr static float radius       = 4000;
        constexpr static float noise_scale  = 5;
        constexpr static float hscale       = 1.2;

        glm::mat4 world2tex;

        public:
        Ground(vec3 offs = vec3(0, -66, 0));
        virtual ~Ground() = default;

        inline void draw_gbuffer(glm::mat4 world2clip, glm::mat4 world2view) override {
            spdlog::debug("here2");
            draw(
                std::vector{prog_defr_ground}, //
                world2clip, world2view, true
            );
        }
        // unused
        inline void draw(
            std::shared_ptr<ShaderProgram> prog, glm::mat4 world2clip, glm::mat4 world2view,
            bool require_sampler
        ) override {
            draw(std::vector{prog}, world2clip, world2view, require_sampler);
        }

        void draw(
            std::vector<std::shared_ptr<ShaderProgram>> progs, glm::mat4 world2clip,
            glm::mat4 world2view, bool require_sampler
        ) override;
    };
} // namespace hmk4_models