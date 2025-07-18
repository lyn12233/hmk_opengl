#include "model.hxx"
#include "mesh.hxx"
#include "texture_objects.hxx"
#include "types.hxx"

#include <algorithm>
#include <assimp/material.h>
#include <assimp/types.h>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iterator>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>

using mf::Model;
using std::shared_ptr;
using std::string;
using std::vector;

Model::Model(string model_file) : model_file_(model_file) {
    if (!model_file_.empty()) {
        setup();
    }
}

Model::~Model() {}

void Model::setup() {
    Assimp::Importer importer;
    const aiScene   *scene =
        importer.ReadFile(model_file_, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        spdlog::error("Model: failed to load {}: {}", model_file_, importer.GetErrorString());
        exit(-1);
    }
    model_dir_ = model_file_.substr(0, model_file_.find_last_of('/'));
    process_node_(scene->mRootNode, scene);
}

string Model::repr() const {
    auto ret = fmt::format("Model at {}: meshes[{}], textures:[", model_file_, meshes.size());
    for (const auto &s : texture_paths) {
        ret += s + ", ";
    }
    ret += "->";
    for (const auto &s : texture_names) {
        ret += s + ", ";
    }
    ret += "] \n\t{\n\t";
    for (const auto &m : meshes) {
        ret += m.repr();
        ret += ",\n\t";
    }
    ret += "}";
    return ret;
}

void Model::process_node_(aiNode *node, const aiScene *scene) {
    spdlog::debug("Model::process_node_");
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(std::move(process_mesh_(mesh, scene)));
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        process_node_(node->mChildren[i], scene);
    }
    spdlog::debug("Model::process_node_: done");
}

mf::Mesh Model::process_mesh_(aiMesh *mesh, const aiScene *scene) {
    spdlog::debug("Model::process_mesh_");
    vector<VertexAttr>                           cur_vertices;
    vector<unsigned>                             cur_indices;
    vector<shared_ptr<glwrapper::TextureObject>> cur_textures;

    for (int i = 0; i < mesh->mNumVertices; i++) {
        VertexAttr vertex;

        auto m_vert = mesh->mVertices[i];
        vertex.pos  = vec3(m_vert.x, m_vert.y, m_vert.z);

        vertex.has_n = mesh->HasNormals();
        if (mesh->HasNormals()) {
            auto m_norm = mesh->mNormals[i];
            vertex.n    = vec3(m_norm.x, m_norm.y, m_norm.z);
        } else {
            vertex.n = vec3(0);
        }

        vertex.has_tan = mesh->HasTangentsAndBitangents();
        if (mesh->HasTangentsAndBitangents()) {
            auto m_tan     = mesh->mTangents[i];
            vertex.tangent = vec3(m_tan.x, m_tan.y, m_tan.z);
            auto m_btan    = mesh->mBitangents[i];
            vertex.tangent = vec3(m_btan.x, m_btan.y, m_btan.z);
        }

        // only use one texture coord?
        vertex.has_tex = mesh->HasTextureCoords(0);
        if (mesh->HasTextureCoords(0)) {
            auto m_tex       = mesh->mTextureCoords[0][i];
            vertex.tex_coord = vec2(m_tex.x, m_tex.y);
        } else {
            vertex.tex_coord = vec2(0);
        }

        vertex.has_c = mesh->HasVertexColors(0);
        if (mesh->HasVertexColors(0)) {
            auto color = mesh->mColors[0][i];
            vertex.c   = vec4(color.r, color.g, color.b, color.a);
        }

        cur_vertices.push_back(vertex);
    } // for (int i = 0; i < mesh->mNumVertices; i++)

    for (int i = 0; i < mesh->mNumFaces; i++) {
        auto face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++)
            cur_indices.push_back(face.mIndices[j]);
    }

    // load textures
    auto   material  = scene->mMaterials[mesh->mMaterialIndex];
    string mesh_name = mesh->mName.C_Str();
    for (auto &s : mesh_name) {
        s = (s == '.' ? '_' : s);
    }

    for (auto [tp, nm] : vector<std::pair<aiTextureType, string>>{
             {aiTextureType_DIFFUSE, mesh_name + ".diffuse"},
             {aiTextureType_SPECULAR, mesh_name + ".specular"},
             {aiTextureType_NORMALS, mesh_name + ".normal"},
             {aiTextureType_METALNESS, mesh_name + ".metalness"}}) {
        auto tex = load_texture_(material, tp, nm, scene);
        cur_textures.insert(cur_textures.end(), tex.begin(), tex.end());
    }

    spdlog::debug("Model::process_mesh_: done");
    return Mesh(cur_vertices, cur_indices, cur_textures, mesh_name);
}

vector<shared_ptr<glwrapper::TextureObject>>
Model::load_texture_(aiMaterial *material, aiTextureType tp, string name, const aiScene *scene) {
    spdlog::info("Model::load_texture_({})", name);
    vector<shared_ptr<glwrapper::TextureObject>> cur_textures;
    for (int i = 0; i < material->GetTextureCount(tp); i++) {
        aiString path;
        if (material->GetTexture(tp, i, &path) != aiReturn_SUCCESS) {
            spdlog::warn("texture not found");
            continue;
        }

        std::shared_ptr<glwrapper::TextureObject> cur_tex;
        // if not texture loaded
        if (std::count(texture_paths.begin(), texture_paths.end(), path.C_Str()) == 0) {
            // empty, need to load
            string s_path = make_string(path);
            // string s_path = path.C_Str();
            spdlog::info("Model::load_texture_(path: {})", s_path);

            // gen tex
            cur_tex = std::make_shared<glwrapper::TextureObject>(
                name, 0, glwrapper::TextureParameter(), GL_RGBA8, GL_TEXTURE_2D
            );

            // is embedded tex
            if (!s_path.empty() && s_path[0] == '*') {
                auto tex = scene->mTextures[std::stoi(s_path.substr(1))];
                // compressed tex
                if (tex->mHeight == 0) {
                    cur_tex->from_image(tex->pcData, tex->mWidth);
                } else {
                    cur_tex->from_data(tex->pcData, tex->mWidth, tex->mHeight);
                }
            } else {
                cur_tex->from_image(s_path);
            }

            // pushback
            texture_paths.push_back(path.C_Str());

        } else { // if texture loaded
            auto idx = std::find(texture_paths.begin(), texture_paths.end(), path.C_Str());
            cur_tex  = textures[std::distance(texture_paths.begin(), idx)];
        } // if not texture loaded

        texture_names.push_back(name);
        textures.push_back(cur_tex);
        cur_textures.push_back(cur_tex);
    }
    spdlog::debug("Model::load_texture_: done");
    return cur_textures;
};