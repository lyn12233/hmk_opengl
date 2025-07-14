#include "checkfail.hxx"
#include "drawable_frame.hxx"
#include "model.hxx"
#include "shader_program.hxx"
#include "widget.hxx"
#include "window.hxx"

#include "debug_struct.hxx"
#include "world_view.hxx"
#include <glm/geometric.hpp>
#include <memory>
#include <spdlog/spdlog.h>

using namespace mf;
using namespace glwrapper;
class MyModel : public WorldViewBase {
    public:
    MyModel() {
        model_ = Model("E:/sch/interest/blender/export/cabin2.glb");
        prog   = std::make_shared<ShaderProgram>("shader_model.vs", "shader_model.fs");
        camera = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 0, 20));
        camera.spin_at_viewpoint_ = false;
    };

    bool draw(DrawableFrame &fbo) override {
        MY_CHECK_FAIL
        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, {0, 255, 0, 1});
        MY_CHECK_FAIL
        fbo.viewport(cur_rect);
        glEnable(GL_DEPTH_TEST);

        for (const auto &mesh : model_.meshes) {
            prog->use();
            prog->set_value("world2clip", camera.world2clip());
            prog->set_value("light_dir", glm::normalize(vec3(-1, -10, -1)));
            prog->set_value("view_pos", camera.viewpoint_);
            mesh.activate_sampler(prog);
            mesh.vao_->bind();

            glDrawElements(GL_TRIANGLES, mesh.indices_.size(), GL_UNSIGNED_INT, 0);
        }
        glDisable(GL_DEPTH_TEST);
        return false;
    }

    std::shared_ptr<ShaderProgram> prog;
    mf::Model                      model_;
};

int main() {
    auto window = std::make_shared<Window>();
    auto world  = std::make_shared<MyModel>();
    spdlog::debug(world->model_.repr());

    window->set_root(world);
    world->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());
    window->mainloop();
}