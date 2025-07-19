#include "buffer_objects.hxx"
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
    MyModel() : g_buffer(800, 600, 4) {
        model_ = Model("E:/sch/interest/blender/export/node2.glb");
        prog   = std::make_shared<ShaderProgram>("shader_defr.vs", "shader_defr.fs");
        camera = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 0, 20));
        camera.spin_at_viewpoint_ = false;
    };

    bool draw(DrawableFrame &fbo) override {
        // MY_CHECK_FAIL
        // fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, {0, 255, 0, 1});
        // MY_CHECK_FAIL
        // fbo.viewport(cur_rect);

        // draw to g_buffer
        g_buffer.bind();
        g_buffer.validate();

        // pos, norm, color.diff, color.spec
        MY_CHECK_FAIL
        const GLenum draw_targ[]{
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4, draw_targ);
        MY_CHECK_FAIL

        // full fbo
        glViewport(0, 0, 800, 600);
        // choose a diff bkgd
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        MY_CHECK_FAIL

        glEnable(GL_DEPTH_TEST);
        MY_CHECK_FAIL

        for (const auto &mesh : model_.meshes) {
            prog->use();
            prog->set_value("world2clip", camera.world2clip());
            mesh.activate_sampler(prog);
            mesh.vao_->bind();

            glDrawElements(GL_TRIANGLES, mesh.indices_.size(), GL_UNSIGNED_INT, 0);
            MY_CHECK_FAIL
        }
        glDisable(GL_DEPTH_TEST);
        MY_CHECK_FAIL

        MY_CHECK_FAIL
        g_buffer.unbind();
        glDrawBuffer(GL_BACK);
        MY_CHECK_FAIL

        // demonstrate normals of g_buffer
        // fbo's size if window's size, turns black at ()
        fbo.paste_tex(g_buffer.tex(1), cur_rect);
        // debug
        // g_buffer.tex(1)->bind();
        // std::vector<glm::vec4> pixel(800 * 600);
        // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel.data());
        // spdlog::debug(::repr(pixel[0]));

        return false;
    }

    std::shared_ptr<ShaderProgram> prog;
    mf::Model                      model_;
    FrameBufferObject              g_buffer;
};

int main() {
    auto window = std::make_shared<Window>();
    spdlog::debug("here");
    auto world = std::make_shared<MyModel>();
    spdlog::debug(world->model_.repr());

    window->set_root(world);
    world->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());
    window->mainloop();
}