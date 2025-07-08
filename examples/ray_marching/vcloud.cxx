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

#define CLOUD_VERTS 10

using namespace glwrapper;

class DrawACloud : public mf::WorldViewBase {
    public:
    DrawACloud() {
        cloud_data = terrain::VolumetricCloudData(
            CLOUD_VERTS, CLOUD_VERTS, CLOUD_VERTS, {1, 2, 3, 4}, {1, 1. / 2, 1. / 4, 1. / 8},
            {0., 0., 0, 1}
        );
        cloud_data.vectorize_inplace([](float x) { return std::max<float>(x, 0.); });
        cloud_data.vectorize_inplace([](float x) { return std::min<float>(x * x * 32, 1); });

        cloud_tex =
            std::make_shared<TextureObject>("", 0, TextureParameter(), GL_R32F, GL_TEXTURE_3D);
        cloud_tex->from_data(
            (void *)cloud_data.data(), CLOUD_VERTS, CLOUD_VERTS, CLOUD_VERTS, GL_FLOAT
        );

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

        camera = mf::WorldCamera(vec3(0, 0, 0), vec3(0, 0, 50));

        offs = vec3(-5, -5, -5);
        x0   = vec3(5, -5, -5);
        y0   = vec3(-5, 5, -5);
        z0   = vec3(-5, -5, 5);

        light_dir = vec3(0, -1, -0.3);

        cloud_light_cache = terrain::gen_light_cache( //
            cloud_data, get_world2tex(offs, x0, y0, z0), light_dir, 20, 10, 1e-2, 4
        );

        cloud_light_tex =
            std::make_shared<TextureObject>("", 0, TextureParameter(), GL_R32F, GL_TEXTURE_3D);
        cloud_light_tex->from_data(
            (void *)cloud_light_cache.data(), cloud_light_cache.shape()[0],
            cloud_light_cache.shape()[1], cloud_light_cache.shape()[2], GL_FLOAT
        );
    }

    bool draw(mf::DrawableFrame &fbo) override {
        // spdlog::debug("DrawACloud::Draw");

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

        prog->set_value("aabb_min", glm::vec3(-5, -5, -5));
        prog->set_value("aabb_max", glm::vec3(5, 5, 5));

        prog->set_value("cloud_world2tex", get_world2tex(offs, x0, y0, z0));
        prog->set_value("light_cache_world2tex", get_world2tex(offs, x0, y0, z0));
        cloud_tex->activate_sampler(prog, "cloud_tex", 0);
        cloud_light_tex->activate_sampler(prog, "light_tex", 1);
        prog->set_value("nb_iter", 10);

        prog->set_value("bkgd_color", glm::vec3(0.53f, 0.81f, 0.92f));
        prog->set_value("light_color", glm::vec3(1.0f, 0.98f, 0.92f));
        prog->set_value("sigma_a", 0.02f);
        prog->set_value("sigma_s", 0.9f);

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

    vec3 offs, x0, y0, z0, light_dir;
};

int main() {
    auto window = std::make_shared<mf::Window>();
    auto sizer  = std::make_shared<mf::BoxSizer>(0, 0, 0, mf::SIZER_HORIZONTAL);
    auto draw   = std::make_shared<DrawACloud>();
    draw->cloud_data.repr();
    draw->cloud_light_cache.repr();

    sizer->add(draw, 1);
    window->set_root(sizer);
    draw->event_at(mf::EVT_FOCUS, mf::Pos(), mf::Rect());

    window->mainloop();
}