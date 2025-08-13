#include "buffer_objects.hxx"
#include "button.hxx"
#include "config.hxx"
#include "drawable_frame.hxx"
#include "model_cloud.hxx"
#include "model_ground.hxx"
#include "model_windgen.hxx"
#include "parameter_dict.hxx"
#include "scene_pipeline.hxx"
#include "shader_program.hxx"
#include "stb_perlin.h"
#include "utils.hxx"
#include "world_view.hxx"

// #include "debug_struct.hxx"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <memory>

using namespace mf;
using namespace glwrapper;
using namespace hmk4_models;

class MyWorld : public WorldViewBase {
    constexpr static int shadow_width = 2000, shadow_height = 2000, nb_gbuffer_att = 5,
                         nb_shadows       = 5;
    constexpr static float windgen_region = 50.f;

    FrameBufferObject                               gbuffer;
    std::vector<std::shared_ptr<FrameBufferObject>> shadow_buffers;
    std::shared_ptr<ParameterDict>                  arguments_;

    std::vector<std::shared_ptr<ModelBase>> models;
    std::shared_ptr<CloudModelBase>         cloud;

    std::vector<glm::vec3> windgen_pos;

    public:
    MyWorld(std::shared_ptr<ParameterDict> arguments) :
        gbuffer(800, 600, nb_gbuffer_att), //
        arguments_(arguments) {

        //
        // init shadow buffers
        for (int i = 0; i < nb_shadows; i++) {
            auto shadow = std::make_shared<FrameBufferObject>(shadow_width, shadow_height, 0);
            shadow_buffers.push_back(shadow);
        }

        //
        // init model
        std::shared_ptr<ModelBase> model;
        model = std::make_shared<Ground>();
        models.push_back(model);
        model = std::make_shared<Windgen>(vec3(0, 0, 0), pi / 4, 0.1);
        models.push_back(model);
        windgen_pos.push_back(vec3(0, 0, 0));

        //
        // init random windgen positions
        for (int i = -3; i < 3; i++) {
            for (int j = i == 0; j < 3; j++) {
                float dx =
                    stb_perlin_noise3_seed(2 * i + 0.5, 2 * j + 0.5, 2 * j + 0.5, 0, 0, 0, 1145);
                float dy =
                    stb_perlin_noise3_seed(2 * i + 0.5, 2 * j + 0.5, 2 * i + 0.5, 0, 0, 0, 11451);
                auto pos    = glm::vec3(i * 300 + dx * 200, 0, j * 300 + dy * 200);
                auto model2 = std::make_shared<Windgen>(
                    pos, pi / 4 + (dx - dy) * 0.1, glm::abs(dx + dy) * 0.4 + 0.2
                );
                models.push_back(model2);
                windgen_pos.push_back(pos);
            }
        }

        //
        // init cloud
        cloud = std::make_shared<Cloud>();

        //
        // camera type
        camera                    = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 20, 20));
        camera.spin_at_viewpoint_ = false;
    }

    bool draw(DrawableFrame &fbo) override {

        spdlog::debug("MyWorld::draw");

        //
        // prepare shadow mapping

        // locate n significant windgen target
        std::vector<float> dists(windgen_pos.size());
        std::vector<int>   idx(windgen_pos.size());

        auto dir = glm::normalize(camera.coord_pos_ - camera.viewpoint_);
        for (int i = 0; i < windgen_pos.size(); i++) {
            idx[i]        = i;
            auto targ_dir = windgen_pos[i] - camera.viewpoint_;
            dists[i]      = glm::length(targ_dir);
            if (glm::acos(glm::dot(dir, glm::normalize(targ_dir))) * 2 >
                    arguments_->get<double>("fov") * camera.perspective_.width_ratio() &&
                dists[i] > windgen_region) {
                dists[i] = 1e6;
            }
        }
        std::sort(idx.begin(), idx.end(), [&](int a, int b) { return dists[a] < dists[b]; });
        // spdlog::info("nearests: {},{}", idx[0], dists[idx[0]]);

        vec3  light_pos = arguments_->get("light.x", "light.y", "light.z");
        float dist      = glm::length(light_pos);

        // global shadow
        std::vector<glm::mat4> n_shadows(nb_shadows);
        n_shadows[nb_shadows - 1] =
            glm::perspective(
                2 * glm::atan(1500.f, glm::length(light_pos)), 1.f, dist - 400.f, dist + 400.f
            ) *
            glm::lookAt(light_pos, vec3(0), vec3(0, 0, 1));

        // shadow surrounding target windgen
        for (int i = 0; i < nb_shadows - 1; i++) {
            dist         = glm::length(light_pos - windgen_pos[idx[i]]);
            n_shadows[i] = glm::perspective(
                               2 * glm::atan(windgen_region, dist), 1.f, dist - windgen_region * 2,
                               dist + windgen_region * 2
                           ) *
                           glm::lookAt(light_pos, windgen_pos[idx[i]], vec3(0, 0, 1));
        }

        //
        // prepare fbo
        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {0, 0, 255, 255});
        fbo.bind();

        //
        // set fov
        camera.perspective_.fovy_ = arguments_->get<double>("fov");

        assert(shadow_buffers.size() == nb_shadows);
        assert(n_shadows.size() == nb_shadows);

        //
        // render pipeline
        hmk4_models::render_scene_defr(
            fbo, cur_rect,                                       //
            models, cloud, gbuffer,                              //
            shadow_buffers, n_shadows, {1e6, 1e4, 1e2, 1, 0.01}, //
            camera.world2clip(), camera.world2view(),            //
            camera.perspective_.fovy_, camera.viewpoint_,        //
            *arguments_
        );

        return false;
    }

    void event_at(EVENT evt, Pos at, EVENT_PARM parameter) override {
        // override resize event
        if (evt == mf::EVT_RESIZE) {
            auto rect = parameter.rect;
            gbuffer   = std::move(FrameBufferObject(
                rect.w * DEFAULT_GBUFFER_SCALING, rect.h * DEFAULT_GBUFFER_SCALING, nb_gbuffer_att
            ));
        }
        WorldViewBase::event_at(evt, at, parameter);
    }
};

static auto inst = std::make_shared<GlfwInst>();

int main() {

    auto window = std::make_shared<mf::Window>((int)(1080 * 1.5), 720, "", inst);

    auto sizer  = std::make_shared<mf::BoxSizer>(0, 0, 0, mf::SIZER_HORIZONTAL);
    auto vsizer = std::make_shared<mf::BoxSizer>(0, 0, 0, mf::SIZER_VERTICAL);

    auto arguments = std::make_shared<ParameterDict>( //
        ParameterDict_t{
            {"light.x", .1},                 //
            {"light.y", 4400.},              //
            {"light.z", 270.},               //
            {"light.r", 1.},                 //
            {"light.g", 1.},                 //
            {"light.b", 1.},                 //
            {"shininess", 32.},              //
            {"s_light", 1.},                 //
            {"fov", glm::pi<double>() / 4.}, //
        }
    );

    auto world = std::make_shared<MyWorld>(arguments);

    auto btn = std::make_shared<Button>("screenshot");

    btn->bind_event(mf::EVT_BUTTON_UP, [&](EVENT, Pos, EVENT_PARM) {
        window->fbo_->do_screenshot(world->cur_rect);
    });

    vsizer->add(world, 1.);
    vsizer->add(btn, 0.);

    sizer->add(vsizer, 1.);
    sizer->add(arguments, 0.3);

    window->set_root(sizer);
    world->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());
    window->draw();
    window->fbo_->do_screenshot(world->cur_rect);
    window->mainloop();
}