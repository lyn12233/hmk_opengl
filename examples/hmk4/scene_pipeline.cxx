#include "scene_pipeline.hxx"
#include "buffer_objects.hxx"
#include "checkfail.hxx"
#include "drawable_frame.hxx"
#include "hmk4_config.hxx"
#include "model.hxx"
#include "shader_program.hxx"
#include "utils.hxx"

#include <memory>
#include <spdlog/spdlog.h>

using namespace hmk4_models;

void ModelBase::draw_gbuffer(glm::mat4 world2clip, glm::mat4 world2view) { assert(false); }
void ModelBase::draw(
    std::vector<std::shared_ptr<ShaderProgram>> progs, glm::mat4 world2clip, glm::mat4 world2view,
    bool require_sampler
) {
    assert(false && "unimplemented");
}
void ModelBase::draw(
    std::shared_ptr<ShaderProgram> prog, glm::mat4 world2clip, glm::mat4 world2view,
    bool require_sampler
) {
    assert(false && "unimplemented");
}
void CloudModelBase::activate_cloud_sampler(std::shared_ptr<ShaderProgram> prog, int at) {
    assert(false && "unimplemented");
}

void hmk4_models::render_scene_defr(                       //
    const mf::DrawableFrame                &fbo,           //
    mf::Rect                                cur_rect,      //
    std::vector<std::shared_ptr<ModelBase>> models,        //
    std::shared_ptr<CloudModelBase>         cloud,         //
    const FrameBufferObject                &gbuffer,       //
    const FrameBufferObject                &shadow_buffer, //
    mat4 world2shadow, mat4 world2clip, mat4 world2view,   //
    float fovy, vec3 view_pos,                             //
    mf::ParameterDict &arguments

) {

    // suppose fbo is cleared

    //
    //
    // init
    spdlog::trace("render_scene_defr: init");

    if (!prog_shade) {
        prog_shade = std::make_shared<ShaderProgram>("shadow_mapping.vs", "shadow_mapping.fs");
    }
    if (!prog_draw) {
        prog_draw = std::make_shared<ShaderProgram>("defr_draw.vs", "defr_draw.fs");
    }
    if (!prog_vis) {
        prog_vis = std::make_shared<ShaderProgram>("defr_vis.vs", "defr_vis.fs");
    }
    if (!vao) {
        vao           = std::make_shared<VertexArrayObject>();
        vbo           = std::make_shared<VertexBufferObject>();
        auto quadvert = std::vector<float>{-1, -1, 1, -1, -1, 1, 1, 1};
        vao->bind();
        {
            vbo->bind();
            vbo->SetBufferData(quadvert.size() * sizeof(float), quadvert.data());
            vbo->SetAttribPointer(0, 2, GL_FLOAT);
        }
        vao->unbind();
    }

    //
    //
    // draw to gbuffer
    spdlog::trace("render_scene_defr: draw to gbuffer");

    gbuffer.bind();
    gbuffer.validate();

    const GLenum draw_targ[]{
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, draw_targ);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, gbuffer.width(), gbuffer.height());

    glEnable(GL_DEPTH_TEST);
    MY_CHECK_FAIL

    for (auto &model : models) {
        model->draw_gbuffer(world2clip, world2view);
    }

    //
    //
    // shadow mapping
    spdlog::debug("render_scene_defr: shadow mapping");

    shadow_buffer.bind();
    glViewport(0, 0, shadow_buffer.width(), shadow_buffer.height());
    glClear(GL_DEPTH_BUFFER_BIT);

    for (auto &model : models) {
        model->draw(prog_shade, world2shadow, mat4(), false);
    }

    shadow_buffer.unbind();
    glDrawBuffer(GL_BACK);

    //
    //
    // calc visibility
    spdlog::trace("render to vis");

    // in: shadowbuffer.depth, gbuffer.pos, world2shadow; out: gbuffer.t_vis(tex(4))
    gbuffer.bind();
    prog_vis->use();
    vao->bind();

    glDrawBuffer(GL_COLOR_ATTACHMENT4);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, gbuffer.width(), gbuffer.height());
    MY_CHECK_FAIL

    glDisable(GL_DEPTH_TEST);

    prog_vis->set_value("world2shadow", world2shadow);
    prog_vis->set_value("world2clip", world2clip);
    prog_vis->set_value("light_pos", arguments.get("light.x", "light.y", "light.z"));

    shadow_buffer.tex_depth()->activate_sampler(prog_vis, "shadow_tex", 0);
    gbuffer.tex(0)->activate_sampler(prog_vis, "gbuffer.t_pos", 1);
    gbuffer.tex(1)->activate_sampler(prog_vis, "gbuffer.t_norm", 2);
    prog_vis->set_value("cursor", (float)arguments.get<double>("cursor"));

    cloud->activate_cloud_sampler(prog_vis, 3);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    vao->unbind();
    MY_CHECK_FAIL

    //
    //
    // draw to frame
    spdlog::debug("render_scene_defr: to frame");

    fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {0, 0, 0, 255});
    prog_draw->use();
    vao->bind();

    MY_CHECK_FAIL
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    MY_CHECK_FAIL

    glDisable(GL_DEPTH_TEST);

    prog_draw->set_value("view_pos", view_pos);
    prog_draw->set_value("light_pos", arguments.get("light.x", "light.y", "light.z"));
    prog_draw->set_value("light_color", arguments.get("light.r", "light.g", "light.b"));
    prog_draw->set_value("s_light", (float)arguments.get<double>("s_light"));
    prog_draw->set_value("shininess", (float)arguments.get<double>("shininess"));

    cloud->activate_cloud_sampler(prog_draw, 6);
    prog_draw->set_value("world2view", world2view);
    prog_draw->set_value("fovy", (float)fovy);

    // prog_draw->set_value("light_world2clip", world2shadow);
    // shadow_buffer.tex_depth()->activate_sampler(prog_draw, "shadow_tex", 0);

    gbuffer.tex(0)->activate_sampler(prog_draw, "gbuffer.t_pos", 1);
    gbuffer.tex(1)->activate_sampler(prog_draw, "gbuffer.t_norm", 2);
    gbuffer.tex(2)->activate_sampler(prog_draw, "gbuffer.t_diff", 3);
    gbuffer.tex(3)->activate_sampler(prog_draw, "gbuffer.t_spec", 4);
    gbuffer.tex(4)->activate_sampler(prog_draw, "gbuffer.t_vis", 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    MY_CHECK_FAIL
    vao->unbind();

    glDisable(GL_DEPTH_TEST);
}