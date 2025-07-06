#pragma once

#include <vector>

namespace terrain{

    class PerlinNoise3D{
        public:
        PerlinNoise3D(int seed, int x_size, int y_size, int z_size);
        float at();

        protected:
        std::vector<std::vector<std::vector<float>>> data_;
    };
    
}