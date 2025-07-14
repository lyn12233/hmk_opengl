#include "mesh.hxx"
#include "buffer_objects.hxx"
#include "texture_objects.hxx"
#include "types.hxx"

#include <memory>
#include <spdlog/spdlog.h>

using glwrapper::BufferObject;
using glwrapper::VertexBufferObject;
using mf::Mesh;

Mesh::Mesh(
    vector<VertexAttr> vertices, vector<unsigned> indices,
    vector<shared_ptr<glwrapper::TextureObject>> textures, string name
) :
    vertices_(vertices),
    indices_(indices), textures_(textures), name_(name) {
    vao_ = std::make_shared<glwrapper::VertexArrayObject>();
    vbo_ = std::make_shared<glwrapper::VertexBufferObject>();
    ebo_ = std::make_shared<glwrapper::BufferObject>(GL_ELEMENT_ARRAY_BUFFER);
    setup();
}

Mesh::~Mesh() {}

void Mesh::setup() {

    vao_->bind();
    vbo_->SetBufferData(vertices_.size() * sizeof(VertexAttr), vertices_.data());
    ebo_->SetBufferData(indices_.size() * sizeof(unsigned), indices_.data());
    auto size = sizeof(VertexAttr);
    vbo_->SetAttribPointer(0, 3, GL_FLOAT, false, size, (void *)offsetof(VertexAttr, pos));
    vbo_->SetAttribPointer(1, 3, GL_FLOAT, false, size, (void *)offsetof(VertexAttr, n));
    vbo_->SetAttribPointer(2, 3, GL_FLOAT, false, size, (void *)offsetof(VertexAttr, tangent));
    vbo_->SetAttribPointer(3, 3, GL_FLOAT, false, size, (void *)offsetof(VertexAttr, bitangent));
    vbo_->SetAttribPointer(4, 2, GL_FLOAT, false, size, (void *)offsetof(VertexAttr, tex_coord));
    vbo_->SetAttribPointer(5, 4, GL_FLOAT, false, size, (void *)offsetof(VertexAttr, c));
    vao_->unbind();
}

std::string Mesh::repr() const {
    auto ret = fmt::format(
        "mf::Mesh<{}>(vertices[{}],indices[{}])", name_, vertices_.size(), indices_.size()
    );
    ret += "{";
    int cnt = 0;
    for (const auto &v : vertices_) {
        ret += ::repr(v) + ",";
        if (cnt++ > 5) {
            ret += "...";
            break;
        }
    }
    ret += "} {";
    cnt = 0;
    for (const auto &i : indices_) {
        ret += fmt::format("{}, ", i);
        if (cnt++ > 5) {
            ret += "...";
            break;
        }
    }
    ret += "} {";
    for (const auto &tex : textures_) {
        ret += tex->name() + ", ";
    }
    ret += "}";
    return ret;
}
