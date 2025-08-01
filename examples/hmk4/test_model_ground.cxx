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
    constexpr static int shadow_width = 4000, shadow_height = 4000, nb_gbuffer_att = 5;

    FrameBufferObject                  gbuffer;
    std::shared_ptr<FrameBufferObject> shadow_buffer;
    std::shared_ptr<FrameBufferObject> shadow_buffer_far;
    std::shared_ptr<ParameterDict>     arguments_;

    std::vector<std::shared_ptr<ModelBase>> models;
    std::shared_ptr<CloudModelBase>         cloud;

    public:
    MyWorld(std::shared_ptr<ParameterDict> arguments) :
        gbuffer(800, 600, nb_gbuffer_att), //
        // shadow_buffer(shadow_width, shadow_height, 0), //
        arguments_(arguments) {

        shadow_buffer     = std::make_shared<FrameBufferObject>(shadow_width, shadow_height, 0);
        shadow_buffer_far = std::make_shared<FrameBufferObject>(shadow_width, shadow_height, 0);

        std::shared_ptr<ModelBase> model;
        model = std::make_shared<Ground>();
        models.push_back(model);
        model = std::make_shared<Windgen>(vec3(0, 0, 0), pi / 4, 0.1);
        models.push_back(model);

        for (int i = -3; i < 3; i++) {
            for (int j = i == 0; j < 3; j++) {
                float dx =
                    stb_perlin_noise3_seed(2 * i + 0.5, 2 * j + 0.5, 2 * j + 0.5, 0, 0, 0, 1145);
                float dy =
                    stb_perlin_noise3_seed(2 * i + 0.5, 2 * j + 0.5, 2 * i + 0.5, 0, 0, 0, 11451);
                auto model2 = std::make_shared<Windgen>(
                    vec3(i * 300 + dx * 200, 0, j * 300 + dy * 200), pi / 4 + (dx - dy) * 0.1,
                    glm::abs(dx + dy) * 0.4 + 0.2
                );
                models.push_back(model2);
            }
        }

        cloud = std::make_shared<Cloud>();

        camera                    = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 20, 20));
        camera.spin_at_viewpoint_ = false;
    }

    bool draw(DrawableFrame &fbo) override {

        spdlog::debug("MyWorld::draw");

        // prepare shadow
        vec3  light_pos = arguments_->get("light.x", "light.y", "light.z");
        vec3  shade_to  = vec3(0);
        float dist      = glm::length(light_pos - shade_to);
        float dist2     = glm::length(light_pos);
        mat4  shadow_mapping =
            glm::perspective<float>(
                2 * glm::atan(60.f, dist), 1.f, glm::max(dist - 100.f, 0.1f), dist + 100.f
            ) *
            glm::lookAt(light_pos, shade_to, vec3(0, 0, 1));
        mat4 shadow_mapping_far =
            glm::perspective<float>(
                2 * glm::atan(1500.f, dist2), 1.f, glm::max(dist2 - 400.f, 0.1f), dist2 + 400.f
            ) *
            glm::lookAt(light_pos, vec3(0), vec3(0, 0, 1));

        // prepare fbo
        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {0, 0, 255, 255});
        fbo.bind();

        // set fov
        camera.perspective_.fovy_ = arguments_->get<double>("fov");

        hmk4_models::render_scene_defr(
            fbo, cur_rect,                                                                        //
            models, cloud, gbuffer,                                                               //
            {shadow_buffer, shadow_buffer_far}, {shadow_mapping, shadow_mapping_far}, {1., 0.01}, //
            camera.world2clip(), camera.world2view(),                                             //
            camera.perspective_.fovy_, camera.viewpoint_,                                         //
            *arguments_
        );

        return false;
    }
    void event_at(EVENT evt, Pos at, EVENT_PARM parameter) override {
        if (evt == mf::EVT_RESIZE) {
            auto rect = parameter.rect;
            gbuffer   = std::move(FrameBufferObject(
                rect.w * DEFAULT_GBUFFER_SCALING, rect.h * DEFAULT_GBUFFER_SCALING, nb_gbuffer_att
            ));
        }
        WorldViewBase::event_at(evt, at, parameter);
    }
};

int main() {
    auto window = std::make_shared<mf::Window>((int)(1080 * 1.5), 720);

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
            {"cursor", 8.},                  // control vis smoothness
            {"s_light", 1.},                 //
            {"fov", glm::pi<double>() / 4.}, //
            // {"scales.ambient", .1},  //
            // {"scales.diffuse", 2.},  //
            // {"scales.specular", .1}, //
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
    window->mainloop();
}