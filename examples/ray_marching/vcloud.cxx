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

#include <memory>

#include <spdlog/spdlog.h>

#define TEX_PIX_PER_METER 2
#define X0 5
#define Y0 5
#define Z0 5

using namespace glwrapper;

class DrawACloud : public mf::WorldViewBase {
    public:
    DrawACloud(std::shared_ptr<mf::ParameterDict> arguments) : arguments(arguments) {
        cloud_tex = std::make_shared<TextureObject>(
            "", 0, TextureParameter(), //
            GL_R32F, GL_TEXTURE_3D
        );
        cloud_light_tex = std::make_shared<TextureObject>(
            "", 0, TextureParameter(), //
            GL_R32F, GL_TEXTURE_3D
        );
        update_cloud_data();
        update_lightcache_data();

        quadvert = std::vector<float>{-1, -1, 1, -1, //
                                      -1, 1,  1, 1};

        vao.bind();
        {
            vbo.bind();
            vbo.SetBufferData(quadvert.size() * sizeof(float), quadvert.data());
            vbo.SetAttribPointer(0, 2, GL_FLOAT);
        }
        vao.unbind();

        prog = std::make_shared<ShaderProgram>("shader_vcloud.vs", "shader_vcloud.fs");

        camera                    = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 0, 50));
        camera.spin_at_viewpoint_ = false;

        offs = vec3(-X0, -Y0, -Z0);
        x0   = vec3(X0, -Y0, -Z0);
        y0   = vec3(-X0, Y0, -Z0);
        z0   = vec3(-X0, -Y0, Z0);

        light_color = glm::vec3(1.0f, 0.98f, 0.92f);

        sigma_a = 0.02;
        sigma_s = 0.98;

    } // constructor

    void update_cloud_data() {
        spdlog::debug("MAIN: update_cloud_data");
        cloud_data = terrain::VolumetricCloudData(
            TEX_PIX_PER_METER * X0, TEX_PIX_PER_METER * Y0 * 2, TEX_PIX_PER_METER * Z0, //
            {1, 2, 3, 4},                                                               //
            {
                (float)std::get<double>((*arguments)["scale1"]), //
                (float)std::get<double>((*arguments)["scale2"]), //
                (float)std::get<double>((*arguments)["scale3"]), //
                (float)std::get<double>((*arguments)["scale4"]), //
            },
            {
                (float)std::get<double>((*arguments)["amp1"]), //
                (float)std::get<double>((*arguments)["amp2"]), //
                (float)std::get<double>((*arguments)["amp3"]), //
                (float)std::get<double>((*arguments)["amp4"]), //
            }
        );

        cloud_data.vectorize_inplace([](float x) { return std::max<float>(x, 0.); });
        cloud_data.vectorize_inplace([](float x) { return std::min<float>(x * x * x * x, 1); });
        // cloud_data.repr();

        MY_CHECK_FAIL
        cloud_tex->from_data(
            (void *)cloud_data.data(),                                                  //
            TEX_PIX_PER_METER * X0, TEX_PIX_PER_METER * Y0 * 2, TEX_PIX_PER_METER * Z0, //
            GL_FLOAT
        );
        MY_CHECK_FAIL
    }

    void update_lightcache_data() {
        spdlog::debug("MAIN: update_lightcache_data");
        light_dir = vec3(
            std::get<double>((*arguments)["light.x"]), //
            std::get<double>((*arguments)["light.y"]), //
            std::get<double>((*arguments)["light.z"])
        );

        cloud_light_cache = terrain::gen_light_cache(               //
            cloud_data, get_world2tex(offs, x0, y0, z0), light_dir, //
            20,                                                     // max_length
            10,                                                     // iter
            std::get<double>((*arguments)["sigma_a"]) +
                std::get<double>((*arguments)["sigma_s"]), // extinction
            4
        );

        // cloud_light_cache.repr();

        spdlog::debug("MAIN: update_lightcache_data (tex)");
        cloud_light_tex->from_data(
            (void *)cloud_light_cache.data(), cloud_light_cache.shape()[0],
            cloud_light_cache.shape()[1], cloud_light_cache.shape()[2], GL_FLOAT
        );
    }

    void update_args() {
        if (arguments->query_content_changed()) {
            spdlog::debug("MAIN: argument changed, resetting");
            update_cloud_data();

            sigma_a = std::get<double>((*arguments)["sigma_a"]);
            sigma_s = std::get<double>((*arguments)["sigma_s"]);

            light_color = vec3( //
                std::get<double>((*arguments)["light.r"]),
                std::get<double>((*arguments)["light.g"]), std::get<double>((*arguments)["light.b"])
            );
            update_lightcache_data();
        }
    }

    bool draw(mf::DrawableFrame &fbo) override {
        spdlog::trace("MAIN: draw {},{},{},{}", cur_rect.x, cur_rect.y, cur_rect.w, cur_rect.h);

        update_args();

        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {0, 0, 0, 0});
        fbo.viewport(cur_rect);

        prog->use();
        vao.bind();

        // set uniforms

        prog->set_value("fovy", camera.perspective_.fovy_);
        prog->set_value(
            "resolution", glm::vec2(camera.perspective_.width_, camera.perspective_.height_)
        );

        prog->set_value("camera_pos", camera.viewpoint_);
        // prog->set_value("camera_dir", camera.viewpoint_ - camera.coord_pos_);
        prog->set_value("world2view", camera.world2view());

        prog->set_value("aabb_min", glm::vec3(-X0, -Y0, -Z0));
        prog->set_value("aabb_max", glm::vec3(X0, Y0, Z0));

        prog->set_value("cloud_world2tex", get_world2tex(offs, x0, y0, z0));
        prog->set_value("light_cache_world2tex", get_world2tex(offs, x0, y0, z0));
        cloud_tex->activate_sampler(prog, "cloud_tex", 0);
        cloud_light_tex->activate_sampler(prog, "light_tex", 1);
        prog->set_value("nb_iter", 10);

        prog->set_value("bkgd_color", glm::vec3(0.53f, 0.81f, 0.92f));
        prog->set_value("light_color", light_color);
        prog->set_value("sigma_a", sigma_a);
        prog->set_value("sigma_s", sigma_s);

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

    terrain::VolumetricCloudData   cloud_data;
    std::shared_ptr<TextureObject> cloud_tex;
    terrain::Array3D<float>        cloud_light_cache;
    std::shared_ptr<TextureObject> cloud_light_tex;

    std::shared_ptr<ShaderProgram> prog;

    std::vector<float> quadvert;
    VertexArrayObject  vao;
    VertexBufferObject vbo;

    vec3 offs, x0, y0, z0, light_dir, light_color;

    float sigma_a, sigma_s;

    std::shared_ptr<mf::ParameterDict> arguments;
};

int main() {
    auto window = std::make_shared<mf::Window>((int)(1080 * 1.5), 720);
    auto sizer  = std::make_shared<mf::BoxSizer>(0, 0, 0, mf::SIZER_HORIZONTAL);
    spdlog::debug("here");
    auto arguments = std::make_shared<mf::ParameterDict>( //
        mf::ParameterDict_t{
            {"scale1", 10.},     //
            {"scale2", 10. / 2}, //
            {"scale3", 10. / 4}, //
            {"scale4", 10. / 8}, //
            {"amp1", 1.},        //
            {"amp2", 1. / 2},    //
            {"amp3", 1. / 4},    //
            {"amp4", 1. / 8},    //
            {"light.x", 0.},     //
            {"light.y", -1.},    //
            {"light.z", 0.3},    //
            {"light.r", 1.00},   //
            {"light.g", 0.98},   //
            {"light.b", 0.92},   //
            {"sigma_a", 0.02},   //
            {"sigma_s", 0.98},   //
        }
    );

    auto draw = std::make_shared<DrawACloud>(arguments);

    draw->cloud_data.repr();
    draw->cloud_light_cache.repr();

    sizer->add(draw, 1);
    sizer->add(arguments, 0.7);
    window->set_root(sizer);
    window->set_focus(draw);

    window->mainloop();
}