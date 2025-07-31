#include "hmk4_config.hxx"

#include "buffer_objects.hxx"
#include "model_ground.hxx"
#include "scene_pipeline.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
#include "types.hxx"
#include "volumetric_cloud.hxx"

#include <glm/matrix.hpp>
#include <memory>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

using namespace hmk4_models;

Ground::Ground(vec3 offs) {

    // init
    prog_defr_ground = std::make_shared<ShaderProgram>("defr_ground.vs", "defr_ground.fs");
    height_map       = std::make_shared<TextureObject>(
        "", 0, TextureParameter("smooth"), GL_R32F, GL_TEXTURE_2D, true
    );

    std::vector<float> verts;
    nb_verts = 0;
    for (float i = -radius; i <= radius; i += chunk_width) {
        for (float j = -radius; j <= radius; j += chunk_height) {
            if (i * i + j * j > radius * radius) continue;

            verts.insert(
                verts.end(),
                {
                    i - chunk_width / 2 + offs.x, offs.y, j - chunk_height / 2 + offs.z, //
                    i - chunk_width / 2 + offs.x, offs.y, j + chunk_height / 2 + offs.z, //
                    i + chunk_width / 2 + offs.x, offs.y, j + chunk_height / 2 + offs.z, //
                    i + chunk_width / 2 + offs.x, offs.y, j + chunk_height / 2 + offs.z, //
                    i + chunk_width / 2 + offs.x, offs.y, j - chunk_height / 2 + offs.z, //
                    i - chunk_width / 2 + offs.x, offs.y, j - chunk_height / 2 + offs.z, //
                }
            );
            nb_verts += 6;
        }
    }
    spdlog::info("Ground::Ground: verts: {}", nb_verts);
    // exit(0);
    vao.bind();
    {
        vbo.bind();
        vbo.SetBufferData(verts.size() * sizeof(float), verts.data());
        vbo.SetAttribPointer(0, 3, GL_FLOAT);
    }
    vao.unbind();

    // gen height map
    auto tex_data = terrain::gen_perlin_tex(
        chunk_height * pix_per_m, chunk_width * pix_per_m, 1, noise_scale, 1145
    );
    tex_data += terrain::gen_perlin_tex(
        chunk_height * pix_per_m, chunk_width * pix_per_m, 1, noise_scale / 4, 114
    );
    tex_data.vectorize_inplace([&](float t) {
        t *= hscale;
        return (glm::exp(t) - glm::exp(-t)) / (glm::exp(t) + glm::exp(-t)) * hscale;
    });
    height_map->from_data(
        tex_data.data(), chunk_width * pix_per_m, chunk_height * pix_per_m, (GLenum)GL_FLOAT
    );

    // get world2tex
    auto u    = glm::vec4(chunk_width, 0, 0, 0);
    auto v    = glm::vec4(0, 0, chunk_height, 0);
    auto w    = glm::vec4(0, 1, 0, 0);
    world2tex = glm::inverse(
        glm::mat4(u, v, w, glm::vec4(offs - vec3(chunk_width / 2, 0, chunk_height / 2), 1))
    );
}

void Ground::draw(
    std::vector<std::shared_ptr<ShaderProgram>> progs, glm::mat4 world2clip, glm::mat4 world2view,
    bool require_sampler
) {
    spdlog::debug("Ground::draw");
    auto prog = progs[0];

    prog->use();
    vao.bind();

    // in : model2clip; world2tex
    prog->set_value("model2clip", world2clip);
    prog->set_value("world2tex", world2tex, true);
    prog->set_value("view_pos", vec3(glm::inverse(world2view) * glm::vec4(0, 0, 0, 1)), true);
    prog->set_value("pix_per_m", pix_per_m, true);
    height_map->activate(0);
    prog->set_value("height_map", (int)0, true);

    glDrawArrays(GL_TRIANGLES, 0, nb_verts);
}
