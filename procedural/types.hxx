#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <stddef.h>
#include <type_traits>
#include <vector>

namespace terrain {
    using std::array;
    using std::vector;
    template<typename T> class Array3D {
        public:
        Array3D<T>(int dimX = 0, int dimY = 0, int dimZ = 0);

        const T &operator[](array<int, 3> offs) const;
        T       &operator[](array<int, 3> offs);

        void vectorize_inplace(std::function<T(T)>);

        array<int, 3> shape() const;

        void *data();

        protected:
        vector<T> data_;

        int dimX_;
        int dimY_;
        int dimZ_;
    };
} // namespace terrain

template<typename T>
terrain::Array3D<T>::Array3D(int dimX, int dimY, int dimZ) :
    data_(vector<T>(dimX * dimY * dimZ)), dimX_(dimX), dimY_(dimY), dimZ_(dimZ) {
    static_assert(!std::is_same_v<T, bool>, "");
}

template<typename T> const T &terrain::Array3D<T>::operator[](array<int, 3> offs) const {
    return const_cast<Array3D<T> *>(this)->operator[](offs);
}
template<typename T> T &terrain::Array3D<T>::operator[](array<int, 3> offs) {
    if (offs[0] < 0) offs[0] += dimX_;
    if (offs[1] < 0) offs[1] += dimY_;
    if (offs[2] < 0) offs[2] += dimZ_;

    assert(0 <= offs[0] && offs[0] < dimX_ && "dimX out of range");
    assert(0 <= offs[1] && offs[1] < dimY_ && "dimY out of range");
    assert(0 <= offs[2] && offs[2] < dimZ_ && "dimZ out of range");

    return data_[dimZ_ * dimY_ * offs[0] + dimY_ * offs[1] + offs[2]];
}

template<typename T> void terrain::Array3D<T>::vectorize_inplace(std::function<T(T)> f) {
    for (auto &i : data_) {
        i = f(i);
    }
}

template<typename T> std::array<int, 3> terrain::Array3D<T>::shape() const {
    return {dimX_, dimY_, dimZ_};
}

template<typename T> void *terrain::Array3D<T>::data() {
    return reinterpret_cast<void *>(data_.data());
}