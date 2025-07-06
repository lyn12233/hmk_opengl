#include "buffer_objects.hxx"
#include "checkfail.hxx"
#include "drawable_frame.hxx"
#include "shader_program.hxx"
#include "widget.hxx"
#include "window.hxx"
#include "world_view.hxx"

#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
    #include "debug_struct.hxx"
#endif

using glm::cos;
using glm::pi;
using glm::sin;

class TheTriangle : public mf::WorldViewBase {
    public:
    TheTriangle() {

        t0 = glfwGetTime();

        std::string vshader = "\
        #version 330 core\n\
        layout(location = 0) in vec2 aPos;\
        layout(location = 1) in vec3 aColor;\
        out vec3 color;\
        uniform mat3 T;\
        uniform mat4 world2clip;\
        void main(){\
            vec3 pos = T * vec3(aPos, 1.0);\
            gl_Position = world2clip * vec4(pos.xy, 1.0, 1.0);\
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
            1,  0,     1, 0, 0, //
            -1, 0,     0, 1, 0, //
            0,  1.732, 0, 0, 1, //
        };

        prog = std::make_shared<ShaderProgram>(vshader, fshader);

        vao.bind();
        vbo.bind();
        vbo.SetBufferData(data.size() * sizeof(float), data.data());
        vbo.SetAttribPointer(0, 2, GL_FLOAT, false, 5 * sizeof(float));
        vbo.SetAttribPointer(1, 3, GL_FLOAT, false, 5 * sizeof(float), (void *)(2 * sizeof(float)));
        vao.unbind();

        camera = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 0, 20));
    }
    bool draw(mf::DrawableFrame &fbo) override {
        // calc t and transform matrix
        float t = (glfwGetTime() - t0);

        t = t - glm::floor(t / (tmove + tspin)) * (tmove + tspin);
        // spdlog::debug("t:{:.3f}",t);

        glm::mat3 mat = {
            1, 0, t * vmove, //
            0, 1, 0,         //
            0, 0, 1,         //
        };

        if (t > tmove) {
            float theta = (t - tmove) * vspin;

            mat = glm::mat3(
                      1, 0, 0,      //
                      0, 1, -yspin, //
                      0, 0, 1       //
                  ) *
                  glm::mat3(
                      cos(theta), -sin(theta), 0, //
                      sin(theta), cos(theta), 0,  //
                      0, 0, 1                     //
                  ) *
                  glm::mat3(
                      1, 0, 0,     //
                      0, 1, yspin, //
                      0, 0, 1      //
                  ) *
                  glm::mat3(
                      1, 0, tmove * vmove, //
                      0, 1, 0,             //
                      0, 0, 1              //
                  );
            /*填充顺序实际为转置*/
        }

        fbo.clear_color(cur_rect);
        fbo.viewport(cur_rect);

        prog->use();
        prog->set_value("T", glm::transpose(mat));
        prog->set_value("world2clip", camera.world2clip());
        vao.bind();

        glDrawArrays(GL_TRIANGLES, 0, 3);

        return false;
    }

    protected:
    std::shared_ptr<ShaderProgram> prog;

    VertexArrayObject  vao;
    VertexBufferObject vbo;
    std::vector<float> data;

    float t0;

    constexpr static float tmove = 1.;
    constexpr static float vmove = 4.;
    constexpr static float tspin = 1.;
    constexpr static float vspin = pi<float>() / 2;
    constexpr static float yspin = 5;
};

int main() {
    auto window   = std::make_shared<mf::Window>();
    auto triangle = std::make_shared<TheTriangle>();
    window->set_root(triangle);
    window->mainloop();
    triangle->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());
}