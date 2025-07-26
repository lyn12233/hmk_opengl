#include "buffer_objects.hxx"
#include "drawable_frame.hxx"
#include "model_windgen.hxx"
#include "parameter_dict.hxx"
#include "scene_pipeline.hxx"
#include "shader_program.hxx"
#include "world_view.hxx"

#include "debug_struct.hxx"
#include <memory>

using namespace mf;
using namespace glwrapper;
using namespace hmk4_models;

class MyWorld : public WorldViewBase {
    constexpr static int shadow_width = 1000, shadow_height = 1000;

    FrameBufferObject              gbuffer;
    FrameBufferObject              shadow_buffer;
    std::shared_ptr<ParameterDict> arguments_;

    std::shared_ptr<Windgen> model;

    public:
    MyWorld(std::shared_ptr<ParameterDict> arguments) :
        gbuffer(800, 600, 4),                          //
        shadow_buffer(shadow_width, shadow_height, 0), //
        arguments_(arguments) {

        model = std::make_shared<Windgen>();

        camera                    = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 0, 20));
        camera.spin_at_viewpoint_ = false;
    }

    bool draw(DrawableFrame &fbo) override {

        spdlog::debug("MyWorld::draw");

        vec3  light_pos = arguments_->get("light.x", "light.y", "light.z");
        float dist      = glm::length(light_pos);
        mat4  shadow_mapping =
            glm::perspective<float>(
                2 * glm::atan(100.f, dist), 1.f, std::max(dist - 100.f, 1e-2f), dist + 100.f
            ) *
            glm::lookAt(light_pos, vec3(0), vec3(0, 1, 0));
        ;

        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {0, 0, 255, 255});
        fbo.bind();

        hmk4_models::render_scene_defr(
            fbo, cur_rect,                                                               //
            {model}, gbuffer, shadow_buffer,                                             //
            shadow_mapping, camera.world2clip(), camera.world2view(), camera.viewpoint_, //
            *arguments_
        );
        spdlog::debug("here");

        return false;
    }
};

int main() {
    auto window = std::make_shared<mf::Window>((int)(1080 * 1.5), 720);

    auto sizer = std::make_shared<mf::BoxSizer>(0, 0, 0, mf::SIZER_HORIZONTAL);

    auto arguments = std::make_shared<ParameterDict>( //
        ParameterDict_t{
            {"light.x", .1},    //
            {"light.y", 44.},   //
            {"light.z", 27.},   //
            {"light.r", 1.},    //
            {"light.g", 1.},    //
            {"light.b", 1.},    //
            {"shininess", 32.}, //
        }
    );

    auto world = std::make_shared<MyWorld>(arguments);

    sizer->add(world, 1.);
    sizer->add(arguments, 0.3);

    window->set_root(sizer);
    world->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());
    window->mainloop();
}