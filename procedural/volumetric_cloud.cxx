#include "volumetric_cloud.hxx"
#include "types.hxx"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <spdlog/spdlog.h>
#include <stb_perlin.h>

using namespace terrain;

VolumetricCloudData::VolumetricCloudData(
    int dimX, int dimY, int dimZ, array<int, nb_level> seed, array<float, nb_level> scale,
    array<float, nb_level> amp
) :
    Array3D<float>(dimX, dimY, dimZ),
    noise_seeds(seed), noise_scales(scale), noise_amps(amp) {

    for (int i = 0; i < dimX; i++) {
        for (int j = 0; j < dimY; j++) {
            for (int k = 0; k < dimZ; k++) {

                float noise = 0;
                for (int l = 0; l < nb_level; l++) {
                    noise += noise_amps[l] *
                             stb_perlin_noise3_seed(
                                 i / scale[l], j / scale[l], k / scale[l], 0, 0, 0, seed[l]
                             );
                }
                (*this)[{i, j, k}] = noise;
            }
        }
    }
}

float terrain::VolumetricCloudData::tex_at(glm::vec3 uvw) const {
    if (std::min<float>({uvw[0], uvw[1], uvw[2]}) <= 0. ||
        std::max<float>({uvw[0], uvw[1], uvw[2]}) >= 1.) {
        return 0.;
    }
    // find voxel
    auto  shape = this->shape();
    float fx    = uvw[0] * (shape[0] - 1);
    float fy    = uvw[1] * (shape[1] - 1);
    float fz    = uvw[2] * (shape[2] - 1);

    int x0 = glm::clamp((int)fx, 0, shape[0] - 1);
    int y0 = glm::clamp((int)fy, 0, shape[1] - 1);
    int z0 = glm::clamp((int)fz, 0, shape[2] - 1);
    int x1 = glm::min(x0 + 1, shape[0] - 1);
    int y1 = glm::min(y0 + 1, shape[1] - 1);
    int z1 = glm::min(z0 + 1, shape[2] - 1);

    // offset in voxel
    float dx = fx - x0;
    float dy = fy - y0;
    float dz = fz - z0;

    // interp with 8 vertices
    float c000 = (*this)[{x0, y0, z0}];
    float c100 = (*this)[{x1, y0, z0}];
    float c010 = (*this)[{x0, y1, z0}];
    float c110 = (*this)[{x1, y1, z0}];
    float c001 = (*this)[{x0, y0, z1}];
    float c101 = (*this)[{x1, y0, z1}];
    float c011 = (*this)[{x0, y1, z1}];
    float c111 = (*this)[{x1, y1, z1}];

    float c00 = glm::mix(c000, c100, dx);
    float c01 = glm::mix(c001, c101, dx);
    float c10 = glm::mix(c010, c110, dx);
    float c11 = glm::mix(c011, c111, dx);

    float c0 = glm::mix(c00, c10, dy);
    float c1 = glm::mix(c01, c11, dy);

    float c = glm::mix(c0, c1, dz);

    return c;
}

Array3D<float> terrain::gen_light_cache(
    const VolumetricCloudData &data, glm::mat4 world2tex, glm::vec3 light_dir, float max_length,
    int nb_iter, float extinction, float sample_rate
) {
    spdlog::debug("terrain::gen_light_cache");

    auto  shape     = data.shape();
    int   dimx      = glm::round(shape[0] / sample_rate);
    int   dimy      = glm::round(shape[1] / sample_rate);
    int   dimz      = glm::round(shape[2] / sample_rate);
    float step_size = max_length / (nb_iter - 1);

    spdlog::debug("dims: ({},{},{})", dimx, dimy, dimz);

    auto result    = Array3D<float>(dimx, dimy, dimz);
    auto tex2world = glm::inverse(world2tex);
    light_dir      = glm::normalize(light_dir);

    float transmittance = 1.;

    for (int i = 0; i < dimx; i++) {
        spdlog::debug("terrain::gen_light_cache: i={}", i);
        for (int j = 0; j < dimy; j++) {
            for (int k = 0; k < dimz; k++) {
                glm::vec3 pos =
                    tex2world * glm::vec4((float)i / dimx, (float)j / dimy, (float)k / dimz, 1.);

                for (int it = 0; it < nb_iter; it++) {
                    auto      sample_pos = pos - light_dir * step_size * (float)(nb_iter - it - 1);
                    glm::vec3 sample_tex_pos = world2tex * glm::vec4(sample_pos, 1.);

                    float density = data.tex_at(sample_tex_pos);

                    transmittance *= glm::exp(-density * extinction * step_size);
                }

                result[{i, j, k}] = transmittance;
            }
        }
    }

    return result;
}