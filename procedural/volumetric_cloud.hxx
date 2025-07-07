#pragma once

#include "types.hxx"

#include <vector>

#include <glm/glm.hpp>

namespace terrain {

    class VolumetricCloudData : public Array3D<float> {
        public:
        constexpr static int nb_level = 4;

        VolumetricCloudData(
            int dimX = 0, int dimY = 0, int dimZ = 0,
            array<int, nb_level>   seed  = array<int, nb_level>{114, 1145, 11451, 1919},
            array<float, nb_level> scale = array<float, nb_level>{8, 4, 2, 1},
            array<float, nb_level> amp   = array<float, nb_level>{8, 4, 2, 1}
        );

        float tex_at(glm::vec3) const;

        array<int, nb_level>   noise_seeds;
        array<float, nb_level> noise_scales;
        array<float, nb_level> noise_amps;
    };

    Array3D<float> gen_light_cache(
        const VolumetricCloudData &data, glm::mat4 world2tex, glm::vec3 light_dir, float max_length,
        int nb_iter = 64, float extinction = 0.1f, float sample_rate = 1.0f
    );
} // namespace terrain