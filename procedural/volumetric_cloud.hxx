#pragma once

#include "types.hxx"

#include <vector>

namespace terrain {
    using std::vector;

    class VolumetricCloudData:public Array3D<float> {
        public:
        constexpr static int nb_level = 4;

        VolumetricCloudData(
            int dimX = 0, int dimY = 0, int dimZ = 0,
            array<int, nb_level>   seed  = array<int, nb_level>{114, 1145, 11451, 1919},
            array<float, nb_level> scale = array<float, nb_level>{8, 4, 2, 1},
            array<float, nb_level> amp   = array<float, nb_level>{8, 4, 2, 1}
        );

        array<int, nb_level>   noise_seeds;
        array<float, nb_level> noise_scales;
        array<float, nb_level> noise_amps;
    };
} // namespace terrain