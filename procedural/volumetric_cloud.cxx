#include "volumetric_cloud.hxx"

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