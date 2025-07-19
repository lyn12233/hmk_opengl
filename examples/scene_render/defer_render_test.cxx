#include "buffer_objects.hxx"
#include "checkfail.hxx"
#include "drawable_frame.hxx"
#include "model.hxx"
#include "parameter_dict.hxx"
#include "shader_program.hxx"
#include "widget.hxx"
#include "window.hxx"
#include "world_view.hxx"

#include <memory>

#include <glm/geometric.hpp>
#include <spdlog/spdlog.h>

#include "debug_struct.hxx"

using namespace mf;
using namespace glwrapper;
class MyModel : public WorldViewBase {
    public:
    MyModel(std::shared_ptr<ParameterDict> arguments) :
        g_buffer(800, 600, 4),        //
        ebo(GL_ELEMENT_ARRAY_BUFFER), //
        arguments(arguments) {

        model_    = Model("E:/sch/interest/blender/export/node2.glb");
        prog_defr = std::make_shared<ShaderProgram>("shader_defr.vs", "shader_defr.fs");
        prog_draw = std::make_shared<ShaderProgram>("shader_scr.vs", "shader_scr.fs");

        camera                    = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 0, 20));
        camera.spin_at_viewpoint_ = false;

        auto quadvert = std::vector<float>{-1, -1, 1, -1, -1, 1, 1, 1};
        vao.bind();
        {
            vbo.bind();
            vbo.SetBufferData(quadvert.size() * sizeof(float), quadvert.data());
            vbo.SetAttribPointer(0, 2, GL_FLOAT);
        }
        vao.unbind();
    };

    bool draw(DrawableFrame &fbo) override {

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
            prog_defr->use();
            prog_defr->set_value("world2clip", camera.world2clip());
            mesh.activate_sampler(prog_defr);
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
        // fbo.paste_tex(g_buffer.tex(1), cur_rect);
        // return false;

        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {0, 0, 0, 0});
        fbo.viewport(cur_rect);
        fbo.bind();

        prog_draw->use();
        vao.bind();

        prog_draw->set_value(
            "resolution", glm::vec2(camera.perspective_.width_, camera.perspective_.height_)
        );

        prog_draw->set_value("view_pos", camera.viewpoint_);
        prog_draw->set_value("light_dir", get("light.x", "light.y", "light.z"));
        prog_draw->set_value("light_color", get("light.r", "light.g", "light.b"));
        prog_draw->set_value("shininess", (float)get<double>("shininess"));

        g_buffer.tex(0)->activate_sampler(prog_draw, "gbuffer.t_pos", 0);
        g_buffer.tex(1)->activate_sampler(prog_draw, "gbuffer.t_norm", 1);
        g_buffer.tex(2)->activate_sampler(prog_draw, "gbuffer.t_diff", 2);
        g_buffer.tex(3)->activate_sampler(prog_draw, "gbuffer.t_spec", 3);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        vao.unbind();

        return false;
    }

    mf::Model                      model_;
    std::shared_ptr<ShaderProgram> prog_defr;
    std::shared_ptr<ShaderProgram> prog_draw;

    FrameBufferObject g_buffer;

    VertexArrayObject  vao;
    VertexBufferObject vbo;
    BufferObject       ebo;

    std::shared_ptr<mf::ParameterDict> arguments;

    template<typename T> T get(std::string v) { return std::get<T>((*arguments)[v]); }
    glm::vec3              get(std::string v1, std::string v2, std::string v3) {
        return glm::vec3(get<double>(v1), get<double>(v2), get<double>(v3));
    }
    glm::vec4 get(std::string v1, std::string v2, std::string v3, std::string v4) {
        return glm::vec4(get<double>(v1), get<double>(v2), get<double>(v3), get<double>(v4));
    }
};

int main() {
    auto window = std::make_shared<mf::Window>((int)(1080 * 1.5), 720);

    auto sizer = std::make_shared<mf::BoxSizer>(0, 0, 0, mf::SIZER_HORIZONTAL);

    auto arguments = std::make_shared<ParameterDict>( //
        ParameterDict_t{
            {"light.x", 0.},    //
            {"light.y", -1.},   //
            {"light.z", 0.},    //
            {"light.r", 1.},    //
            {"light.g", 1.},    //
            {"light.b", 1.},    //
            {"shininess", 32.}, //
        }
    );

    auto world = std::make_shared<MyModel>(arguments);

    spdlog::debug(world->model_.repr());

    sizer->add(world, 1.);
    sizer->add(arguments, 0.3);

    window->set_root(sizer);
    world->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());
    window->mainloop();
}