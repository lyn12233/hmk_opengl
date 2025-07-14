#pragma once

#include "buffer_objects.hxx"
#include "shader_program.hxx"
#include "texture_objects.hxx"
#include "types.hxx"

#include <memory>
#include <vector>

namespace mf {
    using std::shared_ptr;
    using std::string;
    using std::vector;
    class Mesh {
        public:
        Mesh(
            vector<VertexAttr> vertices = {}, vector<unsigned> indices = {},
            vector<shared_ptr<glwrapper::TextureObject>> textures = {}, string name = ""
        );
        ~Mesh();
        void setup();

        inline void activate_sampler(shared_ptr<glwrapper::ShaderProgram> prog) const {
            for (int i = 0; i < textures_.size(); i++) {
                textures_[i]->activate_sampler(prog, "", i);
            }
        }

        std::string repr() const;

        string name_;

        // data
        vector<VertexAttr>                           vertices_;
        vector<unsigned>                             indices_;
        vector<shared_ptr<glwrapper::TextureObject>> textures_;
        vector<string>                               texture_names_; // unused

        // draw utils
        shared_ptr<glwrapper::VertexArrayObject>  vao_;
        shared_ptr<glwrapper::VertexBufferObject> vbo_;
        shared_ptr<glwrapper::BufferObject>       ebo_;
    };
} // namespace mf