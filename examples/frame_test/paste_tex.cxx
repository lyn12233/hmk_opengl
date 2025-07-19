#include "buffer_objects.hxx"
#include "checkfail.hxx"
#include "drawable_frame.hxx"
#include "model.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
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
    MyModel(){};

    bool draw(DrawableFrame &fbo) override {

        auto tex = std::make_shared<TextureObject>();

        auto tex_data = std::vector<glm::vec4>(800 * 600, {1, 0, 1, 1});

        tex->from_data(tex_data.data(), 800, 600, (GLenum)GL_FLOAT, (GLenum)GL_RGBA);

        fbo.paste_tex(tex, cur_rect);
        // tex->repr();

        return false;
    }

    std::shared_ptr<ShaderProgram> prog;
};

int main() {
    auto window = std::make_shared<Window>();
    auto world  = std::make_shared<MyModel>();

    window->set_root(world);
    world->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());
    window->mainloop();
}