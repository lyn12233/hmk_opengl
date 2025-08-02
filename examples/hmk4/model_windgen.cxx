#include "model_windgen.hxx"
#include "model.hxx"
#include "shader.hxx"
#include "shader_program.hxx"

#include <GLFW/glfw3.h>
#include <memory>
#include <spdlog/spdlog.h>

using namespace hmk4_models;
using glm::vec3, glm::cos, glm::sin;

std::shared_ptr<mf::Model> Windgen::turbn_model_ = {};
std::shared_ptr<mf::Model> Windgen::cabin_model_ = {};

Windgen::Windgen(glm::vec3 pos, float phi, float velocity) : //
                                                             // turbn_model_(MODEL_PATH_NODE), //
                                                             // cabin_model_(MODEL_PATH_CABIN), //
    velocity_(velocity) {

    // init
    if (!turbn_model_) {
        turbn_model_ = std::make_shared<mf::Model>(MODEL_PATH_NODE);
    }
    if (!cabin_model_) {
        cabin_model_ = std::make_shared<mf::Model>(MODEL_PATH_CABIN);
    }

    spdlog::debug(
        "#turbine model# \n{}\n#cabin model#\n{}", turbn_model_->repr(), cabin_model_->repr()
    );

    // init
    prog_defr_turbn = std::make_shared<ShaderProgram>("defr_turbn.vs", "defr_turbn.fs");
    prog_defr_cabin = std::make_shared<ShaderProgram>("defr_cabin.vs", "defr_cabin.fs");

    auto vy = vec4(0, 1, 0, 0);
    auto vz = vec4(sin(phi), 0, cos(phi), 0);
    auto vx = vec4(cos(phi), 0, -sin(phi), 0);

    model2world_ = glm::mat4(vx, vy, vz, vec4(pos, 1));

    t0_ = glfwGetTime();
}

void Windgen::draw(
    std::vector<std::shared_ptr<ShaderProgram>> progs, //
    glm::mat4 world2clip, glm::mat4 world2view, bool require_sampler
) {

    // suppose gbuffer is set correct: bind+drawbuffers+viewport+clear+depth_test

    // render cabin
    assert(progs.size() > 1);
    auto prog_turbn = progs[0];
    auto prog_cabin = progs[1];
    for (const auto &mesh : cabin_model_->meshes) {
        prog_cabin->use();
        prog_cabin->set_value("model2clip", world2clip * model2world_);
        prog_cabin->set_value("model2world", model2world_, true); // not used for shadow mapping

        if (require_sampler) mesh.activate_sampler(prog_cabin);
        mesh.vao_->bind();

        glDrawElements(GL_TRIANGLES, mesh.indices_.size(), GL_UNSIGNED_INT, 0);
        MY_CHECK_FAIL
    }

    // render turbine/node

    float dx   = (glfwGetTime() - t0_) * velocity_;
    auto  spin = mat4(
        vec4(0, sin(dx), cos(dx), 0),  //
        vec4(0, cos(dx), -sin(dx), 0), //
        vec4(-1, 0, 0, 0),             //
        vec4(0, 0, 0, 1)
    );
    for (const auto &mesh : turbn_model_->meshes) {
        prog_turbn->use();
        prog_turbn->set_value("model2clip", world2clip * model2world_ * spin);
        prog_turbn->set_value(
            "model2world", model2world_ * spin, true
        ); // not used for shadow mapping

        if (require_sampler) mesh.activate_sampler(prog_turbn);
        mesh.vao_->bind();

        glDrawElements(GL_TRIANGLES, mesh.indices_.size(), GL_UNSIGNED_INT, 0);
        MY_CHECK_FAIL
    }
}