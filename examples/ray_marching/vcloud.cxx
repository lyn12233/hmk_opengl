#include "buffer_objects.hxx"
#include "debug_struct.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
#include "types.hxx"
#include "volumetric_cloud.hxx"
#include "window.hxx"
#include "world_view.hxx"
#include <memory>

class DrawACloud : public mf::WorldViewBase {
    public:
    DrawACloud() {
        cloud_data = terrain::VolumetricCloudData(100, 100, 100);
        cloud_data.vectorize_inplace([](float x) { return std::min<float>(glm::pow(x, 4), 1.); });

        cloud_tex =
            std::make_shared<TextureObject>("", 0, TextureParameter(), GL_R32F, GL_TEXTURE_3D);
        cloud_tex->from_data((GLuint *)cloud_data.data(), 100, 100, 100, GL_FLOAT);

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
            cloud_data, get_world2tex(offs, x0, y0, z0), light_dir, 20, 64, 0.1, 3
        );

        cloud_tex =
            std::make_shared<TextureObject>("", 0, TextureParameter(), GL_R32F, GL_TEXTURE_3D);
        cloud_tex->from_data(
            (GLuint *)cloud_light_cache.data(), cloud_light_cache.shape()[0],
            cloud_light_cache.shape()[1], cloud_light_cache.shape()[2], GL_FLOAT
        );
    }

    bool draw(mf::DrawableFrame &fbo) override {
        spdlog::debug("DrawACloud::Draw");

        fbo.clear_color(cur_rect, GL_COLOR_BUFFER_BIT, {0, 0, 0, 0});
        fbo.viewport(cur_rect);

        prog->use();
        vao.bind();

        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        return false;
    }
    static mat4 get_world2tex(vec3 offs, vec3 x0, vec3 y0, vec3 z0) {
        auto u = glm::vec4(x0 - offs, 1);
        auto v = glm::vec4(y0 - offs, 1);
        auto w = glm::vec4(z0 - offs, 1);
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
    auto draw   = std::make_shared<DrawACloud>();
}