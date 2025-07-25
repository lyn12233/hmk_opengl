// #include "bt.hxx"
#include "buffer_objects.hxx"
#include "checkfail.hxx"
#include "debug_struct.hxx"
#include "parameter_dict.hxx"
#include "shader_program.hxx"
#include "sizer.hxx"
#include "texture_objects.hxx"
#include "types.hxx"
#include "volumetric_cloud.hxx"
#include "window.hxx"
#include "world_view.hxx"

#include <array>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#define TEX_PIX_PER_METER 1
#define X0 100.
#define Y0 10.
#define Z0 100.

using namespace glwrapper;

class DrawACloud : public mf::WorldViewBase {
    public:
    DrawACloud(std::shared_ptr<mf::ParameterDict> arguments) : arguments(arguments) {

        camera                    = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 0, 50));
        camera.spin_at_viewpoint_ = false;

        prog = std::make_shared<ShaderProgram>("shader_vcloud.vs", "shader_vcloud.fs");

        for (auto &tex : perlin_noise_tex) {
            tex =
                std::make_shared<TextureObject>("", 0, TextureParameter(), GL_R32F, GL_TEXTURE_3D);
        }

        update_cloud_data();

        quadvert = std::vector<float>{-1, -1, 1, -1, //
                                      -1, 1,  1, 1};

        vao.bind();
        {
            vbo.bind();
            vbo.SetBufferData(quadvert.size() * sizeof(float), quadvert.data());
            vbo.SetAttribPointer(0, 2, GL_FLOAT);
        }
        vao.unbind();

    } // constructor

    template<typename T> T get(std::string v) { return std::get<T>((*arguments)[v]); }
    glm::vec3              get(std::string v1, std::string v2, std::string v3) {
        return glm::vec3(get<double>(v1), get<double>(v2), get<double>(v3));
    }
    glm::vec4 get(std::string v1, std::string v2, std::string v3, std::string v4) {
        return glm::vec4(get<double>(v1), get<double>(v2), get<double>(v3), get<double>(v4));
    }

    void update_cloud_data() {
        spdlog::debug("MAIN: update_cloud_data");
        lx   = get<double>("aabb.lx");
        ly   = get<double>("aabb.ly");
        lz   = get<double>("aabb.lz");
        offs = vec3(-lx, -ly, -lz);
        x0   = vec3(lx, -ly, -lz);
        y0   = vec3(-lx, ly, -lz);
        z0   = vec3(-lx, -ly, lz);

        for (int i = 0; i < 4; i++) {
            perlin_noise[i] = terrain::gen_perlin_tex(
                lz, ly, lx, get<double>(fmt::format("scale{}", i + 1)), 114 * i
            );
            MY_CHECK_FAIL
            perlin_noise_tex[i]->from_data((void *)perlin_noise[i].data(), lx, ly, lz, GL_FLOAT);
            MY_CHECK_FAIL
        }
    }

    void update_args() {
        if (arguments->query_content_changed()) {
            spdlog::debug("MAIN: argument changed, resetting");
            update_cloud_data();
        }
    }

    bool draw(mf::DrawableFrame &fbo) override {
        spdlog::trace("MAIN: draw {},{},{},{}", cur_rect.x, cur_rect.y, cur_rect.w, cur_rect.h);

        update_args();

        MY_CHECK_FAIL
        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {0, 0, 0, 0});
        MY_CHECK_FAIL
        fbo.viewport(cur_rect);
        MY_CHECK_FAIL
        prog->use();
        MY_CHECK_FAIL
        vao.bind();
        MY_CHECK_FAIL

        // set uniforms

        prog->set_value("fovy", camera.perspective_.fovy_);
        prog->set_value(
            "resolution", glm::vec2(camera.perspective_.width_, camera.perspective_.height_)
        );

        prog->set_value("camera_pos", camera.viewpoint_);
        prog->set_value("world2view", camera.world2view());

        prog->set_value("aabb_min", glm::vec3(-lx, -ly, -lz));
        prog->set_value("aabb_max", glm::vec3(lx, ly, lz));

        prog->set_value("cloud_world2tex", get_world2tex(offs, x0, y0, z0));
        for (int i = 0; i < 4; i++) {
            perlin_noise_tex[i]->activate_sampler(prog, fmt::format("perlin_tex{}", i + 1), i);
            auto name = fmt::format("amp{}", i + 1);
            prog->set_value(name, (float)get<double>(name));
        }
        prog->set_value("density_offs", (float)get<double>("dens_offs"));
        prog->set_value("density_max", (float)get<double>("dens_max"));

        prog->set_value("nb_iter", (int)get<double>("nb_iter"));
        prog->set_value("nb_iter2", (int)get<double>("nb_iter2"));
        prog->set_value("max_length", (float)(4. * std::max({lx, ly, lz})));

        prog->set_value("bkgd_color", glm::vec3(0.53f, 0.81f, 0.92f));
        prog->set_value("light_color", get("light.r", "light.g", "light.b"));
        prog->set_value("sigma_a", (float)get<double>("sigma_a"));
        prog->set_value("sigma_s", (float)get<double>("sigma_s"));
        prog->set_value("light_dir", get("light.x", "light.y", "light.z"));
        prog->set_value(
            "phase_parm",
            get("phase_parm.g1", "phase_parm.g2", "phase_parm.base", "phase_parm.scale")
        );

        MY_CHECK_FAIL

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        return false;
    }
    static mat4 get_world2tex(vec3 offs, vec3 x0, vec3 y0, vec3 z0) {
        auto u = glm::vec4(x0 - offs, 0);
        auto v = glm::vec4(y0 - offs, 0);
        auto w = glm::vec4(z0 - offs, 0);
        auto M = glm::mat4(u, v, w, vec4(offs, 1));
        return glm::inverse(M);
    }

    std::array<terrain::Array3D<float>, 4>        perlin_noise;
    std::array<std::shared_ptr<TextureObject>, 4> perlin_noise_tex;

    std::shared_ptr<ShaderProgram> prog;

    std::vector<float> quadvert;
    VertexArrayObject  vao;
    VertexBufferObject vbo;

    int  lx, ly, lz;
    vec3 offs, x0, y0, z0, light_dir, light_color;

    glm::vec4 phase_parm;

    float sigma_a, sigma_s;

    std::shared_ptr<mf::ParameterDict> arguments;
};

int main() {
    auto window = std::make_shared<mf::Window>((int)(1080 * 1.5), 720);
    auto sizer  = std::make_shared<mf::BoxSizer>(0, 0, 0, mf::SIZER_HORIZONTAL);
    spdlog::debug("here");
    auto arguments = std::make_shared<mf::ParameterDict>( //
        mf::ParameterDict_t{
            {"scale1", 32.},           //
            {"scale2", 16.},           //
            {"scale3", 8.},            //
            {"scale4", 4.},            //
            {"amp1", 1.},              //
            {"amp2", 1. / 2},          //
            {"amp3", 1. / 4},          //
            {"amp4", 1. / 8},          //
            {"light.x", 0.},           //
            {"light.y", -1.},          //
            {"light.z", 0.3},          //
            {"light.r", 1.00},         //
            {"light.g", 0.98},         //
            {"light.b", 0.92},         //
            {"sigma_a", 0.002},        //
            {"sigma_s", 0.098},        //
            {"phase_parm.g1", .83},    //
            {"phase_parm.g2", .3},     //
            {"phase_parm.base", .8},   //
            {"phase_parm.scale", .15}, //
            {"nb_iter", 64.},          //
            {"nb_iter2", 10.},         //
            {"dens_offs", -.3},        //
            {"dens_max", .1},          //
            {"aabb.lx", 100.},         //
            {"aabb.ly", 10.},          //
            {"aabb.lz", 100.},         //
        }
    );

    auto draw = std::make_shared<DrawACloud>(arguments);

    sizer->add(draw, 1);
    sizer->add(arguments, 0.7);
    window->set_root(sizer);
    window->set_focus(draw);

    window->mainloop();
}