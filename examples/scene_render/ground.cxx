#include "buffer_objects.hxx"
#include "parameter_dict.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
#include "types.hxx"
#include "volumetric_cloud.hxx"
#include "window.hxx"
#include "world_view.hxx"

#include <memory>

#include <spdlog/spdlog.h>
#include <vector>

#include "debug_struct.hxx"

using namespace glwrapper;
using namespace mf;

mat4 get_world2tex(vec3 offs, vec3 x0, vec3 y0, vec3 z0);

class MyGround : public WorldViewBase {

    std::shared_ptr<ShaderProgram>     prog;
    std::shared_ptr<mf::ParameterDict> arguments;
    std::shared_ptr<TextureObject>     tex1;
    std::shared_ptr<TextureObject>     tex2;

    constexpr static float plain_width  = 1000;
    constexpr static float plain_height = 1000;
    constexpr static float pix_per_m    = 2.;

    VertexArrayObject  vao;
    VertexBufferObject vbo;

    template<typename T> T get(std::string v) { return std::get<T>((*arguments)[v]); }
    glm::vec3              get(std::string v1, std::string v2, std::string v3) {
        return glm::vec3(get<double>(v1), get<double>(v2), get<double>(v3));
    }
    glm::vec4 get(std::string v1, std::string v2, std::string v3, std::string v4) {
        return glm::vec4(get<double>(v1), get<double>(v2), get<double>(v3), get<double>(v4));
    }

    public:
    MyGround(std::shared_ptr<ParameterDict> arguments) : arguments(arguments) {

        camera                    = WorldCamera(vec3(0), vec3(0, 100, 0));
        camera.spin_at_viewpoint_ = false;

        prog = std::make_shared<ShaderProgram>("ground.vs", "ground.fs");
        tex1 = std::make_shared<TextureObject>("", 0, TextureParameter(), GL_R32F);

        auto vert = std::vector<float>{-1, -1, -plain_width / 2, -plain_height / 2, //
                                       1,  -1, plain_width / 2,  -plain_height / 2, //
                                       -1, 1,  -plain_width / 2, plain_height / 2,
                                       1,  1,  plain_width / 2,  plain_height / 2};
        vao.bind();
        vbo.SetBufferData(vert.size() * sizeof(float), vert.data());
        vbo.SetAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), 0);
        vbo.SetAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        vao.unbind();
    }
    bool draw(mf::DrawableFrame &fbo) override {
        if (arguments->query_content_changed()) {
            auto tex_data = terrain::gen_perlin_tex(
                plain_height * pix_per_m, plain_width * pix_per_m, 1,
                get<double>("scale") * pix_per_m, 1145
            );
            tex1->from_data(
                tex_data.data(), plain_width * pix_per_m, plain_height * pix_per_m, (GLenum)GL_FLOAT
            );
            // tex_data.repr();
            // tex1->repr(1);
            // exit(0);
        }

        auto world2tex = get_world2tex(
            vec3(-plain_width / 2, 0, -plain_height / 2),
            vec3(plain_width / 2, 0, -plain_height / 2),
            vec3(-plain_width / 2, 0, plain_height / 2),
            vec3(-plain_width / 2, 1, -plain_height / 2)
        );
        // spdlog::debug(::repr(world2tex));

        fbo.clear_color(cur_rect);
        prog->use();
        vao.bind();

        prog->set_value("world2clip", camera.world2clip());
        prog->set_value("world2tex", world2tex);

        prog->set_value("c_diff", get("c1.r", "c1.g", "c1.b"));
        prog->set_value("c_spec", get("c2.r", "c2.g", "c2.b"));

        prog->set_value("light_color", get("light.r", "light.g", "light.b"));
        prog->set_value("light_pos", get("light.x", "light.y", "light.z"));
        prog->set_value("shininess", (float)get<double>("shininess"));

        prog->set_value("view_pos", camera.viewpoint_);

        tex1->activate_sampler(prog, "tex1", 0);
        prog->set_value("hscale", (float)get<double>("hscale"));

        prog->set_value("pix_per_m", (float)pix_per_m);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        return false;
    }
};

int main() {
    auto window    = std::make_shared<mf::Window>((int)(1080 * 1.5), 720);
    auto sizer     = std::make_shared<mf::BoxSizer>(0, 0, 0, mf::SIZER_HORIZONTAL);
    auto arguments = std::make_shared<mf::ParameterDict>(mf::ParameterDict_t{
        {"c1.r", 1.},       //
        {"c1.g", 1.},       //
        {"c1.b", 1.},       //
        {"c2.r", 1.},       //
        {"c2.g", 1.},       //
        {"c2.b", 1.},       //
        {"light.r", 1.},    //
        {"light.g", 1.},    //
        {"light.b", 1.},    //
        {"light.x", 1.},    //
        {"light.y", 1e5},   //
        {"light.z", 1.},    //
        {"shininess", 32.}, //
        {"scale", 5.},      //
        {"hscale", 1.}      //
    });

    auto draw = std::make_shared<MyGround>(arguments);

    sizer->add(draw, 1);
    sizer->add(arguments, 0.7);
    window->set_root(sizer);
    window->set_focus(draw);

    window->mainloop();
}

mat4 get_world2tex(vec3 offs, vec3 x0, vec3 y0, vec3 z0) {
    auto u = glm::vec4(x0 - offs, 0);
    auto v = glm::vec4(y0 - offs, 0);
    auto w = glm::vec4(z0 - offs, 0);
    auto M = glm::mat4(u, v, w, vec4(offs, 1));
    return glm::inverse(M);
}