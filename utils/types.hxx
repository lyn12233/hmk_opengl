#pragma once

#include "types.hxx"
#include <array>
#include <cassert>
#include <fmt/format.h>
#include <functional>
#include <map>
#include <stddef.h>
#include <type_traits>
#include <variant>
#include <vector>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

namespace terrain {
    using glm::vec4;
    using std::array;
    using std::vector;
    template<typename T> class Array3D {
        public:
        Array3D<T>(int dimX = 0, int dimY = 0, int dimZ = 0);
        void operator=(vector<T>);
        void operator+=(const Array3D<T> &);
        void operator*=(const Array3D<T> &);
        void operator*=(const T &);

        const T &operator[](array<int, 3> offs) const;
        T       &operator[](array<int, 3> offs);

        void vectorize_inplace(std::function<T(T)>);
        void vectorize_inplace(std::function<T(T, int, int, int)>);

        array<int, 3> shape() const;

        void *data();

        void repr();

        protected:
        vector<T> data_;

        int dimX_;
        int dimY_;
        int dimZ_;
    };
} // namespace terrain

namespace mf {
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;
    typedef std::map<std::string, std::variant<int, float>> ShaderArgumentDict;
    struct VertexAttr {
        vec3 pos, n, tangent, bitangent;
        vec2 tex_coord;
        vec4 c;
        bool has_n : 1;
        bool has_tan : 1;
        bool has_tex : 1;
        bool has_c : 1;
    };
} // namespace mf

template<typename T> std::string repr(const T &var) {
    if constexpr (std::is_same_v<T, glm::mat4>) {
        glm::mat4 v = var;
        return fmt::format(
            "mat4[\n\t{}\n\t{}\n\t{}\n\t{}\n]", repr(v[0]), repr(v[1]), repr(v[2]), repr(v[3])
        );
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
        glm::vec4 v = var;
        return fmt::format("vec4[{},{},{},{}]", v.x, v.y, v.z, v.w);
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
        glm::vec3 v = var;
        return fmt::format("vec3[{},{},{}]", v.x, v.y, v.z);
    } else if constexpr (std::is_same_v<T, mf::VertexAttr>) {
        mf::VertexAttr v = var;
        return fmt::format(
            "vertex({:.3f},{:.3f},{:.3f};{:.3f},{:.3f};{}{}{})", v.pos.x, v.pos.y, v.pos.z,
            v.tex_coord.x, v.tex_coord.y, "n"[!v.has_n], "t"[!v.has_tex], "c"[!v.has_c]
        );
    }
    return "";
}

// implementations

template<typename T>
terrain::Array3D<T>::Array3D(int dimX, int dimY, int dimZ) :
    data_(vector<T>(dimX * dimY * dimZ)), dimX_(dimX), dimY_(dimY), dimZ_(dimZ) {
    static_assert(!std::is_same_v<T, bool>, "");
}

template<typename T> void terrain::Array3D<T>::operator=(vector<T> vec) {
    assert(vec.size() == dimX_ * dimY_ * dimZ_ && "Array3D<T>::operator=: invalid size");
    data_ = vec;
}
template<typename T> void terrain::Array3D<T>::operator+=(const Array3D<T> &o) {
    assert(dimX_ == o.dimX_ && dimY_ == o.dimY_ && dimZ_ == o.dimZ_);
    for (int i = 0; i < data_.size(); i++) {
        data_[i] += o.data_[i];
    }
}
template<typename T> void terrain::Array3D<T>::operator*=(const Array3D<T> &o) {
    assert(dimX_ == o.dimX_ && dimY_ == o.dimY_ && dimZ_ == o.dimZ_);
    for (int i = 0; i < data_.size(); i++) {
        data_[i] *= o.data_[i];
    }
}
template<typename T> void terrain::Array3D<T>::operator*=(const T &t) {
    for (int i = 0; i < data_.size(); i++) {
        data_[i] *= t;
    }
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

    return data_[dimZ_ * dimY_ * offs[0] + dimZ_ * offs[1] + offs[2]];
}

template<typename T> void terrain::Array3D<T>::vectorize_inplace(std::function<T(T)> f) {
    for (auto &i : data_) {
        i = f(i);
    }
}
template<typename T>
void terrain::Array3D<T>::vectorize_inplace(std::function<T(T, int, int, int)> f) {
    for (int i = 0; i < dimX_; i++) {
        for (int j = 0; j < dimY_; j++) {
            for (int k = 0; k < dimZ_; k++) {
                T val = data_[dimZ_ * dimY_ * i + dimZ_ * j + k];

                data_[dimZ_ * dimY_ * i + dimZ_ * j + k] = f(val, i, j, k);
            }
        }
    }
}

template<typename T> std::array<int, 3> terrain::Array3D<T>::shape() const {
    return {dimX_, dimY_, dimZ_};
}

template<typename T> void *terrain::Array3D<T>::data() {
    return reinterpret_cast<void *>(data_.data());
}

template<typename T> void terrain::Array3D<T>::repr() {
    spdlog::info("Array3D<> ({},{},{})", (int)dimX_, (int)dimY_, (int)dimZ_);
    assert(data_.size() == dimX_ * dimY_ * dimZ_ && "data_ size wrong");

    std::string s;
    s = "[";
    for (int x = 0; x < dimX_; ++x) {
        s += " [";
        for (int y = 0; y < dimY_; ++y) {
            s += "  [";
            for (int z = 0; z < dimZ_; ++z) {
                s += std::to_string((*this)[{x, y, z}]);
                if (z != dimZ_ - 1) s += " ";
            }
            s += "]";
            if (y != dimY_ - 1) s += " ";
        }
        s += "]";
        if (x != dimX_ - 1) s += " ";
    }
    s += "]";
    spdlog::info(s);
}