#pragma once

#include "hmk4_config.hxx"

#include "buffer_objects.hxx"
#include "model.hxx"
#include "scene_pipeline.hxx"
#include "shader_program.hxx"

#include <memory>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

namespace hmk4_models {
    using namespace glwrapper;
    constexpr float pi = glm::pi<float>();
    class Windgen : public ModelBase {
        public:
        mf::Model turbn_model_;
        mf::Model cabin_model_;

        std::shared_ptr<ShaderProgram> prog_defr_turbn;
        std::shared_ptr<ShaderProgram> prog_defr_cabin;

        // glm::mat4 model2world_;
        float velocity_;
        float t0_;

        public:
        Windgen(glm::vec3 pos = vec3(0, 0, 0), float phi = 0, float velocity = 1.);
        virtual ~Windgen() = default;

        inline void draw_gbuffer(glm::mat4 world2clip, glm::mat4 world2view) override {
            draw({prog_defr_turbn, prog_defr_cabin}, world2clip, world2view, true);
        }
        // unused
        inline void draw(
            std::shared_ptr<ShaderProgram> prog, glm::mat4 world2clip, glm::mat4 world2view,
            bool require_sampler
        ) override {
            draw({prog, prog}, world2clip, world2view, require_sampler);
        }

        void draw(
            std::vector<std::shared_ptr<ShaderProgram>> progs, glm::mat4 world2clip,
            glm::mat4 world2view, bool require_sampler
        ) override;
    };
} // namespace hmk4_models