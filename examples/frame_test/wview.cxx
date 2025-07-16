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
#include <string>

using namespace mf;
using namespace glwrapper;

class SimpleView : public WorldViewBase {
    public:
    SimpleView() : ebo(GL_ELEMENT_ARRAY_BUFFER) {}
    bool draw(DrawableFrame &fbo) override {

        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, {0, 255, 0, 1});
        MY_CHECK_FAIL
        fbo.viewport(cur_rect);
        glEnable(GL_DEPTH_TEST);

        // draw

        glDisable(GL_DEPTH_TEST);
        return false;
    }
    VertexArrayObject  vao;
    VertexBufferObject vbo;
    BufferObject       ebo;
    ShaderProgram      prog;
};

int main() {}