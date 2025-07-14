#pragma once

#include "buffer_objects.hxx"
#include "mesh.hxx"
#include "texture_objects.hxx"
#include "types.hxx"

#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/types.h>
#include <memory>
#include <vector>

#include <assimp/scene.h>

namespace mf {
    using std::string;
    class Model {
        public:
        Model(string model_file = "");
        ~Model();
        void setup();

        string repr() const;

        // data
        vector<shared_ptr<glwrapper::TextureObject>> textures;
        vector<Mesh>                                 meshes;
        vector<string>                               texture_paths, texture_names;
        string                                       model_dir_, model_file_;

        protected:
        void process_node_(aiNode *node, const aiScene *scene);
        Mesh process_mesh_(aiMesh *mesh, const aiScene *scene);

        vector<shared_ptr<glwrapper::TextureObject>>
        load_texture_(aiMaterial *material, aiTextureType tp, string name, const aiScene *scene);

        private:
        inline string make_string(aiString s) {
            char *x = const_cast<char *>(s.C_Str());
            if (x == nullptr) return "<null>";

            int i = 0;
            while (x[i] == '\0') {
                i++;
            }
            return string(&x[i]);
        };
    };
} // namespace mf