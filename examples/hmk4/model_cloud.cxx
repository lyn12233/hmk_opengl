#include "model_cloud.hxx"
#include "checkfail.hxx"
#include "types.hxx"
#include "volumetric_cloud.hxx"
#include <array>
#include <glm/geometric.hpp>

using namespace hmk4_models;

Cloud::Cloud(vec3 aabb_min, vec3 aabb_max, float pix_per_m, int seed1, int seed2) {
    spdlog::debug("Cloud::Cloud");
    aabb_min_ = glm::min(aabb_min, aabb_max);
    aabb_max_ = glm::max(aabb_max, aabb_min);
    assert(aabb_min_.x != aabb_max_.x);
    assert(aabb_min_.y != aabb_max_.y);
    assert(aabb_min_.z != aabb_max_.z);

    pix_per_m_ = pix_per_m;

    // gen tex
    auto box     = (aabb_max_ - aabb_min_);
    auto tex_box = box * pix_per_m_;

    auto noise_data =
        terrain::Array3D<float>((int)tex_box.z + 1, (int)tex_box.y + 1, (int)tex_box.x + 1);
    for (int i = 0; i < 4; i++) {
        auto noise = terrain::gen_perlin_tex(
            (int)tex_box.z + 1, (int)tex_box.y + 1, (int)tex_box.x + 1, glm::pow(2.f, -i) * 10,
            seed1
        );
        noise *= glm::pow(2.f, -i);
        noise_data += noise;
    }
    noise_data.vectorize_inplace([&](float v, int i, int j, int k) {
        float x = k / tex_box.x * 2 - 1, y = j / tex_box.y * 2 - 1, z = i / tex_box.z * 2 - 1;
        v *= glm::pow(glm::max<float>(1 - x * x - z * z, 0), 1. / 8.);
        v *= glm::min<float>(3 * (y - 1), 1);
        v *= glm::min<float>(1 * (y + 1), 1);
        v = glm::pow(glm::max<float>(v - 0.3, 0), 2);
        v = v > 1.8 ? glm::mix<float>(2, 1.8, glm::exp(-v + 0.8)) : v;
        v *= 0.05;
        return (float)v;
    });
    spdlog::debug("here");
    // cloud material
    sigma_a_ = 0.002;
    sigma_s_ = 0.098;

    // get world2tex
    world2tex_ = mat4(              //
        vec4(1 / (box.x), 0, 0, 0), //
        vec4(0, 1 / (box.y), 0, 0), //
        vec4(0, 0, 1 / (box.z), 0), //
        vec4(-aabb_min_ / box, 1)
    );

    // gen tex
    tex_ = std::make_shared<TextureObject>( //
        "", 0, TextureParameter("smooth"), GL_R32F, GL_TEXTURE_3D
    );
    tex_->from_data(
        (void *)noise_data.data(), (int)tex_box.x + 1, (int)tex_box.y + 1, (int)tex_box.z + 1,
        GL_FLOAT
    );
    MY_CHECK_FAIL
    // spdlog::debug("Cloud: cloud data:");
    // noise_data.repr();
    // exit(0);
}

void Cloud::activate_cloud_sampler(std::shared_ptr<ShaderProgram> prog, int at) {
    // spdlog::debug("Cloud::activate_cloud_sampler");
    prog->set_value("cloud.aabb_min", aabb_min_);
    prog->set_value("cloud.aabb_max", aabb_max_);
    prog->set_value("cloud.sigma_a", sigma_a_);
    prog->set_value("cloud.sigma_s", sigma_s_);
    prog->set_value("cloud.world2tex", world2tex_);
    tex_->activate_sampler(prog, "cloud.tex", at);
    // exit(0);
}
