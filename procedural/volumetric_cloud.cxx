#include "volumetric_cloud.hxx"
#include "types.hxx"

#include <array>
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

    array<float, nb_level> offset;
    for (int l = 0; l < nb_level; l++) {
        offset[l] = stb_perlin_noise3_seed(.5, .5, .5, 0, 0, 0, seed[l]) * 0.1 + 0.5;
    }

    for (int i = 0; i < dimX; i++) {
        for (int j = 0; j < dimY; j++) {
            for (int k = 0; k < dimZ; k++) {

                float noise = 0;
                for (int l = 0; l < nb_level; l++) {
                    noise += noise_amps[l] * //
                             stb_perlin_noise3_seed(
                                 (i + offset[l]) / scale[l], (j + offset[l]) / scale[l],
                                 (k + offset[l]) / scale[l], 0, 0, 0, seed[l]
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

    assert(step_size > 1e-6 && step_size < 1e6);

    spdlog::debug("dims: ({},{},{})", dimx, dimy, dimz);

    auto result    = Array3D<float>(dimx, dimy, dimz);
    auto tex2world = glm::inverse(world2tex);

    glm::vec3 rd = glm::normalize(light_dir);

    for (int i = 0; i < dimx; i++) {
        spdlog::debug("terrain::gen_light_cache: i={}", i);
        for (int j = 0; j < dimy; j++) {
            for (int k = 0; k < dimz; k++) {
                // spdlog::debug(
                // "{}-1*{}", repr(world2tex),
                // repr(glm::vec4((float)i / dimx, (float)j / dimy, (float)k / dimz, 1.))
                // );

                glm::vec3 ro =
                    tex2world * glm::vec4((float)i / dimx, (float)j / dimy, (float)k / dimz, 1.);
                float transmittance = 1.;

                for (int it = 0; it < nb_iter; it++) {
                    // spdlog::debug("ro:{},rd:{},step_size:{}", repr(ro), repr(rd), step_size);
                    auto sample_pos = ro - rd * step_size * (float)(nb_iter - it - 1);
                    // spdlog::debug("sample_pos: {}", repr(glm::vec4(sample_pos, 1)));

                    glm::vec3 sample_tex_pos = world2tex * glm::vec4(sample_pos, 1.);

                    float density = data.tex_at(sample_tex_pos);
                    assert(density < 1e6);

                    transmittance *= glm::exp(-density * extinction * step_size);
                }

                result[{i, j, k}] = transmittance;
            }
        }
    }

    return result;
}

Array3D<float> terrain::gen_perlin_tex(int dimX, int dimY, int dimZ, float noise_scale, int seed) {
    Array3D<float> array{dimX, dimY, dimZ};

    for (int i = 0; i < dimX; i++) {
        for (int j = 0; j < dimY; j++) {
            for (int k = 0; k < dimZ; k++) {
                float offset =
                    stb_perlin_noise3_seed(i / 10., j / 10., k / 10., 0, 0, 0, seed + 5) * 0.5 +
                    0.5;

                array[{i, j, k}] = stb_perlin_noise3_seed(
                    (i + offset) / noise_scale, (j + offset) / noise_scale,
                    (k + offset) / noise_scale, 0, 0, 0, seed
                );
            }
        }
    }

    return array;
}