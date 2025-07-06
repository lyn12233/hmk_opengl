#include "buffer_objects.hxx"
#include "checkfail.hxx"
#include "drawable_frame.hxx"
#include "shader_program.hxx"
#include "utils.hxx"
#include "widget.hxx"
#include "window.hxx"
#include "world_view.hxx"

#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#ifdef _DEBUG
    #include "debug_struct.hxx"
#endif

class TheTriangle : public mf::WorldViewBase {
    public:
    TheTriangle() {
        std::string vshader = "\
        #version 330 core\n\
        layout(location = 0) in vec2 aPos;\
        layout(location = 1) in vec3 aColor;\
        out vec3 color;\
        void main(){\
            gl_Position = vec4(aPos, 1.0, 1.0);\
            color = aColor;\
        }\
        ";

        std::string fshader = "\
        #version 330 core\n\
        in vec3 color;\
        out vec4 FragColor;\
        void main(){\
            FragColor = vec4(color, 1.0);\
        }\
        ";

        data = {
            // vertices  //color
            .7,  -.7, 1, 0, 0, //
            -.7, -.7, 0, 1, 0, //
            .0,  .7,  0, 0, 1, //
        };

        prog = std::make_shared<ShaderProgram>(vshader, fshader);

        vao.bind();
        vbo.bind();
        vbo.SetBufferData(data.size() * sizeof(float), data.data());
        vbo.SetAttribPointer(0, 2, GL_FLOAT, false, 5 * sizeof(float));
        vbo.SetAttribPointer(1, 3, GL_FLOAT, false, 5 * sizeof(float), (void *)(2 * sizeof(float)));
        vao.unbind();
    }
    bool draw(mf::DrawableFrame &fbo) override {
        fbo.clear_color(cur_rect);
        fbo.viewport(cur_rect);

        prog->use();
        vao.bind();

        glDrawArrays(GL_TRIANGLES, 0, 3);

        return false;
    }

    protected:
    std::shared_ptr<ShaderProgram> prog;

    VertexArrayObject  vao;
    VertexBufferObject vbo;
    std::vector<float> data;
};

int main() {
    auto window   = std::make_shared<mf::Window>();
    auto triangle = std::make_shared<TheTriangle>();
    window->set_root(triangle);
    window->mainloop();
    triangle->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());
}