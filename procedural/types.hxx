#pragma once

#include <array>
#include <cassert>
#include <stddef.h>
#include <vector>

namespace terrain {
    using std::array;
    using std::vector;
    template<typename T> class Array3D {
        public:
        Array3D<T>(int dimX=0, int dimY=0, int dimZ=0);

        T& operator[](array<int, 3> offs);

        array<int,3> shape();

        vector<T> data_;
        int       dimX_;
        int       dimY_;
        int       dimZ_;
    };
} // namespace terrain

template<typename T>
terrain::Array3D<T>::Array3D(int dimX, int dimY, int dimZ)
    : data_(vector<T>(dimX * dimY * dimZ)), dimX_(dimX), dimY_(dimY), dimZ_(dimZ) {}

template<typename T> T& terrain::Array3D<T>::operator[](array<int, 3> offs) {
    if (offs[0] < 0) offs[0] += dimX_;
    if (offs[1] < 0) offs[1] += dimX_;
    if (offs[2] < 0) offs[2] += dimY_;

    assert(0 <= offs[0] && offs[0] < dimX_ && "dimX out of range");
    assert(0 <= offs[1] && offs[1] < dimY_ && "dimY out of range");
    assert(0 <= offs[2] && offs[2] < dimZ_ && "dimZ out of range");

    return data_[dimZ_ * dimY_ * offs[0] + dimY_ * offs[1] + offs[2]];
}

template<typename T>
std::array<int,3> terrain::Array3D<T>::shape(){return {dimX_,dimY_,dimZ_};}